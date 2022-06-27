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
#include "mavengenerator.h"
#include "mavenasynparse.h"
#include "mavenitemkeeper.h"
#include "transceiver/sendevents.h"
#include "transceiver/projectmavenreceiver.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"

#include <QtXml>
#include <QFileIconProvider>

class MavenGeneratorPrivate
{
    friend class MavenGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *mavenMenu {nullptr};
    QHash<QStandardItem*, MavenAsynParse*> projectParses {};
};

MavenGenerator::MavenGenerator()
    : d(new MavenGeneratorPrivate())
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (!builderService) {
        qCritical() << "Failed, not found service : builderService";
        abort();
    }

    QObject::connect(this, &ProjectGenerator::targetExecute,
                     [=](const QString &cmd, const QStringList &args) {
        emit builderService->builderCommand(cmd, args);
    });
}

MavenGenerator::~MavenGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QWidget *MavenGenerator::configureWidget(const QString &language,
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

    QString projectFilePath = projectPath;

    // get config result.

    ProjectInfo info;
    QString sourceFolder = QFileInfo(projectPath).path();
    info.setLanguage(language);
    info.setSourceFolder(sourceFolder);
    info.setKitName(MavenGenerator::toolKitName());
    info.setWorkspaceFolder(sourceFolder);
    info.setProjectFilePath(projectFilePath);

    configure(info);

    return nullptr;
}

bool MavenGenerator::configure(const dpfservice::ProjectInfo &info)
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

    Generator::started(); // emit starded
    return true;
}

QStandardItem *MavenGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;

    QStandardItem * rootItem = new QStandardItem(QFileInfo(info.sourceFolder()).fileName());
    dpfservice::ProjectInfo::set(rootItem, info);
    d->projectParses[rootItem] = new MavenAsynParse();
    QObject::connect(d->projectParses[rootItem], &MavenAsynParse::parsedProject,
                     this, &MavenGenerator::doProjectAddRows, Qt::UniqueConnection);
    d->projectParses[rootItem]->parseProject(info);

    return rootItem;
}

void MavenGenerator::removeRootItem(QStandardItem *root)
{
    recursionRemoveItem(root);
}

QMenu *MavenGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    QMenu *menu = new QMenu();
    if (!d->mavenMenu) {
        d->mavenMenu = new QMenu("Maven", menu);
        QObject::connect(d->mavenMenu, &QMenu::destroyed,
                         [=](){ d->mavenMenu = nullptr; });
    }

    menu->addMenu(d->mavenMenu);

    // asyn parse
    using namespace dpfservice;

    QStandardItem *itemTemp = const_cast<QStandardItem *>(item);
    if (!itemTemp)
        return d->mavenMenu;

    auto parse = d->projectParses[itemTemp];
    if (!parse)
        return d->mavenMenu;

    ProjectInfo info = ProjectInfo::get(item);
    if (info.isEmpty())
        return d->mavenMenu;

    // add menu generat call back
    QObject::connect(parse, &MavenAsynParse::parsedActions,
                     this, &MavenGenerator::doAddMavenMeue,
                     Qt::UniqueConnection);
    // execute logic
    parse->parseActions(info);

    return menu;
}

void MavenGenerator::doProjectAddRows(const dpfservice::ParseInfo<QList<QStandardItem *>> &info)
{
    auto rootItem = d->projectParses.key(qobject_cast<MavenAsynParse*>(sender()));
    if (rootItem)
        rootItem->appendRows(info.result);
}

void MavenGenerator::doAddMavenMeue(const dpfservice::ParseInfo<dpfservice::ProjectActionInfos> &info)
{
    if (d->mavenMenu) {
        for (auto actionInfo : info.result) {
            QAction *action = new QAction(actionInfo.displyText, d->mavenMenu);
            dpfservice::ProjectActionInfo::set(action, actionInfo);
            d->mavenMenu->addAction(action);
        }
    }
}

void MavenGenerator::doActionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        dpfservice::ProjectActionInfo::get(action);
    }
}
