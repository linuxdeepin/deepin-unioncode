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
#include "ninjaprojectgenerator.h"
#include "ninjaasynparse.h"

#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

#include <QtConcurrent>
#include <QtXml>
#include <QFileIconProvider>

class NinjaProjectGeneratorPrivate
{
    friend class NinjaProjectGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *ninjaMenu {nullptr};
    QProcess *menuGenProcess {nullptr};
    QHash<QStandardItem*, NinjaAsynParse*> projectParses {};
};

NinjaProjectGenerator::NinjaProjectGenerator()
    : d(new NinjaProjectGeneratorPrivate())
{
    qRegisterMetaType<QList<QStandardItem*>>("QList<QStandardItem*>");
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

NinjaProjectGenerator::~NinjaProjectGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QStringList NinjaProjectGenerator::supportLanguages()
{
    return {dpfservice::MWMFA_CXX};
}

QStringList NinjaProjectGenerator::supportFileNames()
{
    return {"build.ninja"};
}

QDialog *NinjaProjectGenerator::configureWidget(const QString &language,
                                          const QString &projectPath)
{
    using namespace dpfservice;

    ProjectInfo info;
    info.setLanguage(language);
    info.setKitName(NinjaProjectGenerator::toolKitName());
    info.setWorkspaceFolder(projectPath);

    configure(info);

    return nullptr;
}

bool NinjaProjectGenerator::configure(const dpfservice::ProjectInfo &info)
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

    dpfservice::ProjectGenerator::configure(info);

    return true;
}

QStandardItem *NinjaProjectGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    QStandardItem * rootItem = ProjectGenerator::createRootItem(info);
    dpfservice::ProjectInfo::set(rootItem, info);
    d->projectParses[rootItem] = new NinjaAsynParse();
    QObject::connect(d->projectParses[rootItem], &NinjaAsynParse::itemsModified,
                     this, &NinjaProjectGenerator::doProjectChildsModified);
    QMetaObject::invokeMethod(d->projectParses[rootItem], "parseProject",
                              Q_ARG(const dpfservice::ProjectInfo &, info));
    return rootItem;
}

void NinjaProjectGenerator::removeRootItem(QStandardItem *root)
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

QMenu *NinjaProjectGenerator::createItemMenu(const QStandardItem *item)
{
    Q_UNUSED(item)
    return nullptr;
}

void NinjaProjectGenerator::doProjectChildsModified(const QList<QStandardItem *> &items)
{
    auto rootItem = d->projectParses.key(qobject_cast<NinjaAsynParse*>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(items);
    }
}
