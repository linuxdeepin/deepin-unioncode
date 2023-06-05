/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "pythonprojectgenerator.h"
#include "pythonasynparse.h"
#include "python/project/properties/configpropertywidget.h"
#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

#include <QtConcurrent>
#include <QtXml>
#include <QFileIconProvider>

class PythonProjectGeneratorPrivate
{
    friend class PythonProjectGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *pythonMenu {nullptr};
    QProcess *menuGenProcess {nullptr};
    QHash<QStandardItem*, PythonAsynParse*> projectParses {};
};

PythonProjectGenerator::PythonProjectGenerator()
    : d(new PythonProjectGeneratorPrivate())
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

PythonProjectGenerator::~PythonProjectGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QStringList PythonProjectGenerator::supportLanguages()
{
    return {dpfservice::MWMFA_PYTHON};
}

QDialog *PythonProjectGenerator::configureWidget(const QString &language,
                                          const QString &projectPath)
{
    using namespace dpfservice;


    ProjectInfo info;
    info.setLanguage(language);
    info.setKitName(PythonProjectGenerator::toolKitName());
    info.setWorkspaceFolder(projectPath);

    configure(info);

    return nullptr;
}

bool PythonProjectGenerator::configure(const dpfservice::ProjectInfo &info)
{
    dpfservice::ProjectGenerator::configure(info);

    auto root = createRootItem(info);
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && root) {
        projectService->projectView.addRootItem(root);
        projectService->projectView.expandedDepth(root, 1);
    }

    return true;
}

QStandardItem *PythonProjectGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;

    QStandardItem * rootItem = ProjectGenerator::createRootItem(info);
    dpfservice::ProjectInfo::set(rootItem, info);
    d->projectParses[rootItem] = new PythonAsynParse();
    QObject::connect(d->projectParses[rootItem],
                     &PythonAsynParse::itemsModified,
                     this, &PythonProjectGenerator::doProjectChildsModified,
                     Qt::ConnectionType::UniqueConnection);
    d->projectParses[rootItem]->parseProject(info);

    return rootItem;
}

void PythonProjectGenerator::removeRootItem(QStandardItem *root)
{
    if (!root)
        return;
    auto parser = d->projectParses[root];

    while (root->hasChildren()) {
        root->takeRow(0);
    }
    d->projectParses.remove(root);

    delete root;

    if (parser)
        delete parser;
}

QMenu *PythonProjectGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    QMenu *menu = new QMenu();
    dpfservice::ProjectInfo info = dpfservice::ProjectInfo::get(item);
    if (info.isEmpty())
        return nullptr;

    QStandardItem *itemTemp = const_cast<QStandardItem *>(item);
    if (!itemTemp)
        return nullptr;

    QAction *action = new QAction(tr("Properties"));
    menu->addAction(action);
    QObject::connect(action, &QAction::triggered, [=](){
        actionProperties(info, itemTemp);
    });

    return menu;
}

void PythonProjectGenerator::doProjectChildsModified(const QList<QStandardItem *> &info)
{
    auto rootItem = d->projectParses.key(qobject_cast<PythonAsynParse*>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(info);
    }
}

void PythonProjectGenerator::doPythonCleanMenu()
{
    if (d->pythonMenu) {
        for (auto &action : d->pythonMenu->actions()) {
            d->pythonMenu->removeAction(action);
        }
    }
}

void PythonProjectGenerator::actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;
    ConfigPropertyWidget *property = new ConfigPropertyWidget(info, item);
    dlg.insertPropertyPanel("Config", property);
    dlg.exec();
}
