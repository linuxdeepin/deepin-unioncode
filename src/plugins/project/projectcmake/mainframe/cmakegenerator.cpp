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
#include "cmakegenerator.h"
#include "cmakeasynparse.h"
#include "cmakeitemkeeper.h"
#include "transceiver/projectcmakereceiver.h"
#include "properties/buildpropertywidget.h"
#include "properties/runpropertywidget.h"
#include "properties/configpropertywidget.h"
#include "properties/configutil.h"
#include "properties/targetsmanager.h"

#include "services/builder/builderservice.h"
#include "services/window/windowservice.h"
#include "common/dialog/propertiesdialog.h"

#include <QtXml>
#include <QFileIconProvider>
#include <QPushButton>

class CmakeGeneratorPrivate
{
    friend class CmakeGenerator;

    enum CreateItemMode{
        NewCreateProject,
        RebuildProject,
    };

    QHash<QStandardItem*, QThreadPool*> asynItemThreadPolls;
    QList<QStandardItem*> reloadCmakeFileItems;
    dpfservice::ProjectInfo configureProjectInfo;
};

CmakeGenerator::CmakeGenerator()
    : d(new CmakeGeneratorPrivate())
{
    QObject::connect(this, &CmakeGenerator::createRootItemAsynEnd,
                     this, &CmakeGenerator::setRootItemToView);

    // when execute command end can create root Item
    QObject::connect(ProjectCmakeProxy::instance(),
                     &ProjectCmakeProxy::buildExecuteEnd,
                     this, &CmakeGenerator::doBuildCmdExecuteEnd);

    // main thread init watcher class
    CmakeItemKeeper::instance();

    // build cmake file changed notify
    QObject::connect(CmakeItemKeeper::instance(),
                     &CmakeItemKeeper::cmakeFileNodeNotify,
                     this, &CmakeGenerator::doCmakeFileNodeChanged);

    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (!builderService) {
        qCritical() << "Failed, not found service : builderService";
        abort();
    }
}

CmakeGenerator::~CmakeGenerator()
{
    qInfo() << __FUNCTION__;
    for (auto val : d->asynItemThreadPolls.keys()) {
        auto threadPoll = d->asynItemThreadPolls[val];
        if (threadPoll) {
            threadPoll->clear();
            while (threadPoll->activeThreadCount() != 0) {}
            delete threadPoll;
        }
    }

    d->asynItemThreadPolls.clear();

    if (d)
        delete d;
}

QStringList CmakeGenerator::supportLanguages()
{
    return {dpfservice::MWMFA_CXX};
}

QStringList CmakeGenerator::supportFileNames()
{
    return {"cmakelists.txt", "CMakeLists.txt"};
}

QDialog *CmakeGenerator::configureWidget(const QString &language,
                                         const QString &workspace)
{
    ProjectGenerator::configureWidget(language, workspace);

    config::ConfigureParam *param = config::ConfigUtil::instance()->getConfigureParamPointer();
    if (!config::ConfigUtil::instance()->isNeedConfig(workspace, *param)) {
        dpfservice::ProjectInfo info;
        if (config::ConfigUtil::instance()->updateProjectInfo(info, param)) {
            configure(info);
            TargetsManager::instance()->initialize(info.buildFolder());
            return nullptr;
        }
    }

    // show build type config pane.
    ConfigPropertyWidget *configPropertyWidget = new ConfigPropertyWidget(language, workspace);
    QObject::connect(config::ConfigUtil::instance(), &config::ConfigUtil::configureDone,
                     [this](const dpfservice::ProjectInfo &info) {
        configure(info);
    });

    return configPropertyWidget;
}

bool CmakeGenerator::configure(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {
        // cache project info, asyn end to use
        d->configureProjectInfo = info;

        BuildCommandInfo commandInfo;
        commandInfo.kitName = info.kitName();
        commandInfo.program = info.buildProgram();
        commandInfo.arguments = info.configCustomArgs();
        commandInfo.workingDir = info.workspaceFolder();

        ProjectCmakeProxy::instance()->setBuildCommandUuid(commandInfo.uuid);
        builderService->interface.builderCommand(commandInfo);

        // TODO(logan): change CDT4_GENERATOR to CBP_GENERATOR
#if 0
        BuildCommandInfo cbpCommandInfo;
        cbpCommandInfo.kitName = info.kitName();
        cbpCommandInfo.program = info.buildProgram();
        commandInfo.arguments[5] = CDT_PROJECT_KIT::get()->CBP_GENERATOR;
        cbpCommandInfo.arguments << commandInfo.arguments;
        cbpCommandInfo.workingDir = info.workspaceFolder();
        builderService->interface.builderCommand(commandInfo);
#endif
    }

    dpfservice::ProjectGenerator::configure(info);

    return true;
}

QStandardItem *CmakeGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    QStandardItem * rootItem = ProjectGenerator::createRootItem(info);

    d->asynItemThreadPolls[rootItem] = new QThreadPool;

    auto parse = new CmakeAsynParse;

    // asyn free parse, that .project file parse
    QObject::connect(parse, &CmakeAsynParse::parseProjectEnd,
                     [=](CmakeAsynParse::ParseInfo<QStandardItem*> info){
        d->asynItemThreadPolls.remove(info.result);
        delete parse;
        createRootItemAsynEnd(info.result);
    });

    // asyn execute logic,  that .project file parse
    QtConcurrent::run(d->asynItemThreadPolls[rootItem],
                      parse, &CmakeAsynParse::parseProject,
                      rootItem, info);

    return rootItem;
}

void CmakeGenerator::removeRootItem(QStandardItem *root)
{
    // remove watcher from current root item
    CmakeItemKeeper::instance()->delCmakeFileNode(root);

    auto threadPoll = d->asynItemThreadPolls[root];
    if (threadPoll) {
        threadPoll->clear();
        while(threadPoll->waitForDone());
        delete threadPoll;
        d->asynItemThreadPolls.remove(root);
    }

    recursionRemoveItem(root);
}

QMenu *CmakeGenerator::createItemMenu(const QStandardItem *item)
{
    QMenu *menu = nullptr;

    // create parse
    CmakeAsynParse *parse = new CmakeAsynParse();

    // create item from syn
    auto targetBuilds = parse->parseActions(item);

    // free parse from syn
    delete parse;

    auto root = ProjectGenerator::root(const_cast<QStandardItem *>(item));
    if (!root)
        return menu;

    if (!targetBuilds.isEmpty()) {
        menu = new QMenu();
        for (auto val : targetBuilds) {
            QAction *action = new QAction();
            action->setText(val.buildName);
            action->setProperty("workDir", dpfservice::ProjectInfo::get(root).workspaceFolder());
            action->setProperty(CDT_CPROJECT_KEY::get()->buildCommand.toLatin1(), val.buildCommand);
            action->setProperty(CDT_CPROJECT_KEY::get()->buildArguments.toLatin1(), val.buildArguments);
            action->setProperty(CDT_CPROJECT_KEY::get()->buildTarget.toLatin1(), val.buildTarget);
            action->setProperty(CDT_CPROJECT_KEY::get()->stopOnError.toLatin1(), val.stopOnError);
            action->setProperty(CDT_CPROJECT_KEY::get()->useDefaultCommand.toLatin1(), val.useDefaultCommand);
            QObject::connect(action, &QAction::triggered, this, &CmakeGenerator::actionTriggered, Qt::UniqueConnection);
            menu->addAction(action);
        }
    }

    if (!menu) {
        menu = new QMenu();
    }

    QAction *action = new QAction(tr("Properties"));
    menu->addAction(action);
    dpfservice::ProjectInfo info = dpfservice::ProjectInfo::get(item);
    QObject::connect(action, &QAction::triggered, [=](){
        actionProperties(info, root);
    });

    return menu;
}

void CmakeGenerator::actionTriggered()
{
    using namespace dpfservice;
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        QString program = action->property(CDT_CPROJECT_KEY::get()->buildCommand.toLatin1()).toString();
        QStringList args = action->property(CDT_CPROJECT_KEY::get()->buildArguments.toLatin1()).toString().split(" ");
        args << action->property(CDT_CPROJECT_KEY::get()->buildTarget.toLatin1()).toString();
        QString workDir = action->property("workDir").toString();

        // remove extra quotes and empty argument.
        QStringList argsFiltered;
        for (auto &arg : args) {
            if (!arg.isEmpty()) {
                argsFiltered << arg.replace("\"", "");
            }
        }

        using namespace dpfservice;
        auto &ctx = dpfInstance.serviceContext();
        BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
        if (builderService) {
            BuildCommandInfo commandInfo;
            commandInfo.kitName = CmakeGenerator::toolKitName();
            commandInfo.program = program;
            commandInfo.arguments = args;
            commandInfo.workingDir = workDir;
            builderService->interface.builderCommand(commandInfo);
        }
    }
}

void CmakeGenerator::setRootItemToView(QStandardItem *root)
{
    d->asynItemThreadPolls.remove(root);

    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return;

    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    if (root) {
        // setting item to view
        if (projectService->projectView.addRootItem)
            projectService->projectView.addRootItem(root);

        // expand view from tree two level
        if (projectService->projectView.expandedDepth)
            projectService->projectView.expandedDepth(root, 2);

        navigation.doSwitch(MWNA_EDIT);
        editor.switchWorkspace(MWCWT_PROJECTS);
    }
}

void CmakeGenerator::doBuildCmdExecuteEnd(const BuildCommandInfo &info, int status)
{
    // configure function cached info
    if (d->configureProjectInfo.isEmpty())
        return;

    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return;

    // get reload item from reload cmake file cache
    mutex.lock();
    QStandardItem *reloadItem = nullptr;
    for (auto val : d->reloadCmakeFileItems) {
        if(info.program == d->configureProjectInfo.buildProgram()
                && info.arguments == d->configureProjectInfo.buildCustomArgs()) {
            reloadItem = val;
            break;
        }
    }
    mutex.unlock();

    if (reloadItem) {
        d->reloadCmakeFileItems.removeOne(reloadItem); //clean cache
        if (status == 0) {
            projectService->projectView.removeRootItem(reloadItem);
            createRootItem(d->configureProjectInfo);
        } else {
            qCritical() << "Failed execute cmd : "
                        << info.program
                        << info.arguments.join(" ")
                        << "status : " << status;
        }
    } else {
        createRootItem(d->configureProjectInfo);
    }

    //d->configureProjectInfo = {};

    emit projectService->projectConfigureDone(QString());
}

void CmakeGenerator::doCmakeFileNodeChanged(QStandardItem *root, const QPair<QString, QStringList> &files)
{
    Q_UNUSED(files)

    if (d->reloadCmakeFileItems.contains(root))
        return;

    qInfo() << __FUNCTION__;
    using namespace dpfservice;

    // get current project info
    auto proInfo = ProjectInfo::get(root);

    // cache the reload item
    d->reloadCmakeFileItems.append(root);

    // reconfigure project info
    configure(proInfo);
}

void CmakeGenerator::actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;

    BuildPropertyWidget *buildWidget = new BuildPropertyWidget(info);
    RunPropertyWidget *runWidget = new RunPropertyWidget(info, item);

    dlg.insertPropertyPanel(tr("Build"), buildWidget);
    dlg.insertPropertyPanel(tr("Run"), runWidget);

    dlg.exec();
}

void CmakeGenerator::recursionRemoveItem(QStandardItem *item)
{
    if (!item)
        return;

    for (int row = 0; row < item->rowCount(); row ++) {
        auto child = item->takeChild(row);
        if (!child->hasChildren()) {
            delete child;
        } else {
            recursionRemoveItem(child);
        }
    }

    delete item;
    return;
}
