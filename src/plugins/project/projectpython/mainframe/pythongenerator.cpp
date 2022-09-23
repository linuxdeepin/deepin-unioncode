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
#include "pythongenerator.h"
#include "pythonasynparse.h"
#include "pythonitemkeeper.h"
#include "transceiver/sendevents.h"
#include "transceiver/projectpythonreceiver.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

#include <QtConcurrent>
#include <QtXml>
#include <QFileIconProvider>

class PythonGeneratorPrivate
{
    friend class PythonGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *gradleMenu {nullptr};
    QProcess *menuGenProcess {nullptr};
    QHash<QStandardItem*, PythonAsynParse*> projectParses {};
};

PythonGenerator::PythonGenerator()
    : d(new PythonGeneratorPrivate())
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

PythonGenerator::~PythonGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QWidget *PythonGenerator::configureWidget(const QString &language,
                                          const QString &projectPath)
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return nullptr;

    auto proInfos = projectService->projectView.getAllProjectInfo();
    for (auto &val : proInfos) {
        if (val.language() == language && projectPath == val.projectFilePath()) {
            ContextDialog::ok(QDialog::tr("Cannot open repeatedly!\n"
                                          "language : %0\n"
                                          "projectPath : %1")
                              .arg(language, projectPath));
            return nullptr;
        }
    }

    ProjectInfo info;
    info.setLanguage(language);
    info.setSourceFolder(projectPath);
    info.setKitName(PythonGenerator::toolKitName());
    info.setWorkspaceFolder(projectPath);
    info.setProjectFilePath(projectPath);

    configure(info);

    return nullptr;
}

bool PythonGenerator::configure(const dpfservice::ProjectInfo &info)
{
    auto root = createRootItem(info);
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (projectService && windowService && root) {
        projectService->projectView.addRootItem(root);
        projectService->projectView.expandedDepth(root, 1);
        windowService->switchWidgetNavigation(MWNA_EDIT);
        windowService->switchWidgetWorkspace(MWCWT_PROJECTS);
    }

    dpfservice::ProjectGenerator::configure(info);

    return true;
}

QStandardItem *PythonGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;

    QStandardItem * rootItem = new QStandardItem(QFileInfo(info.sourceFolder()).fileName());
    dpfservice::ProjectInfo::set(rootItem, info);
    d->projectParses[rootItem] = new PythonAsynParse();
    QObject::connect(d->projectParses[rootItem],
                     &PythonAsynParse::itemsModified,
                     this, &PythonGenerator::doProjectChildsModified,
                     Qt::ConnectionType::UniqueConnection);
    d->projectParses[rootItem]->parseProject(info);

    return rootItem;
}

void PythonGenerator::removeRootItem(QStandardItem *root)
{
    if (!root)
        return;

    auto parse = d->projectParses[root];
    if (parse)
        parse->removeRows();
    delete root;
    d->projectParses.remove(root);
}

QMenu *PythonGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    return nullptr;
}

void PythonGenerator::doProjectChildsModified(const dpfservice::ParseInfo<QList<QStandardItem *> > &info)
{
    auto rootItem = d->projectParses.key(qobject_cast<PythonAsynParse*>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(info.result);
    }
}

void PythonGenerator::doPythonCleanMenu()
{
    if (d->gradleMenu) {
        for (auto &action : d->gradleMenu->actions()) {
            d->gradleMenu->removeAction(action);
        }
    }
}
