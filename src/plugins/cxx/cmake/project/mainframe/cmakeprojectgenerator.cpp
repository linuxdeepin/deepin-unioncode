// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakeprojectgenerator.h"
#include "cmakeasynparse.h"
#include "cmakeitemkeeper.h"
#include "cmake/project/transceiver/projectcmakereceiver.h"
#include "properties/buildpropertywidget.h"
#include "properties/runpropertywidget.h"
#include "properties/configpropertywidget.h"
#include "properties/configutil.h"
#include "properties/targetsmanager.h"
#include "services/builder/builderservice.h"
#include "services/window/windowservice.h"
#include "common/dialog/propertiesdialog.h"
#include "common/util/eventdefinitions.h"

#include <QtXml>
#include <QFileIconProvider>
#include <QPushButton>

class CmakeProjectGeneratorPrivate
{
    friend class CmakeProjectGenerator;

    enum CreateItemMode {
        NewCreateProject,
        RebuildProject,
    };

    QHash<QStandardItem *, QThreadPool *> asynItemThreadPolls;
    QList<QStandardItem *> reloadCmakeFileItems;
    dpfservice::ProjectInfo configureProjectInfo;
};

CmakeProjectGenerator::CmakeProjectGenerator()
    : d(new CmakeProjectGeneratorPrivate())
{
    // when execute command end can create root Item
    QObject::connect(ProjectCmakeProxy::instance(),
                     &ProjectCmakeProxy::buildExecuteEnd,
                     this, &CmakeProjectGenerator::doBuildCmdExecuteEnd);

    // main thread init watcher class
    CmakeItemKeeper::instance();

    // build cmake file changed notify
    QObject::connect(CmakeItemKeeper::instance(),
                     &CmakeItemKeeper::cmakeFileNodeNotify,
                     this, &CmakeProjectGenerator::doCmakeFileNodeChanged);

    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (!builderService) {
        qCritical() << "Failed, not found service : builderService";
        abort();
    }
}

CmakeProjectGenerator::~CmakeProjectGenerator()
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

QStringList CmakeProjectGenerator::supportLanguages()
{
    return { dpfservice::MWMFA_CXX };
}

QStringList CmakeProjectGenerator::supportFileNames()
{
    return { "cmakelists.txt", "CMakeLists.txt" };
}

QDialog *CmakeProjectGenerator::configureWidget(const QString &language,
                                         const QString &workspace)
{
    ProjectGenerator::configureWidget(language, workspace);

    config::ConfigureParam *param = config::ConfigUtil::instance()->getConfigureParamPointer();
    if (!config::ConfigUtil::instance()->isNeedConfig(workspace, *param)) {
        dpfservice::ProjectInfo info;
        if (config::ConfigUtil::instance()->updateProjectInfo(info, param)) {
            configure(info);
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

bool CmakeProjectGenerator::configure(const dpfservice::ProjectInfo &projInfo)
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {


        BuildCommandInfo commandInfo;
        commandInfo.kitName = projInfo.kitName();
        commandInfo.program = projInfo.buildProgram();
        commandInfo.arguments = projInfo.configCustomArgs();
        commandInfo.workingDir = projInfo.workspaceFolder();

        bool isSuccess = builderService->interface.builderCommand({commandInfo}, false);
        if (isSuccess) {
            ProjectCmakeProxy::instance()->setBuildCommandUuid(commandInfo.uuid);
            // display root item before everything is done.
            rootItem = ProjectGenerator::createRootItem(projInfo);
            setRootItemToView(rootItem);

            dpfservice::ProjectGenerator::configure(projInfo);

            // cache project info, asyn end to use
            d->configureProjectInfo = projInfo;

            return true;
        }
    }
    return false;
}

QStandardItem *CmakeProjectGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;

    d->asynItemThreadPolls[rootItem] = new QThreadPool;

    auto parse = new CmakeAsynParse;

    // asyn free parse, that .project file parse
    QObject::connect(parse, &CmakeAsynParse::parseProjectEnd,
                     [=](CmakeAsynParse::ParseInfo<QStandardItem *> parseInfo) {
                         d->asynItemThreadPolls.remove(parseInfo.result);
                         // active after everything done.
                         project.activeProject(info.kitName(), info.language(), info.workspaceFolder());
                         delete parse;
                     });

    // asyn execute logic,  that .project file parse
    QtConcurrent::run(d->asynItemThreadPolls[rootItem],
                      parse, &CmakeAsynParse::parseProject,
                      rootItem, info);

    return rootItem;
}

void CmakeProjectGenerator::removeRootItem(QStandardItem *root)
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

QMenu *CmakeProjectGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

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
            QObject::connect(action, &QAction::triggered, this, &CmakeProjectGenerator::actionTriggered, Qt::UniqueConnection);
            menu->addAction(action);
        }
    }

    if (!menu) {
        menu = new QMenu();
    }

    QAction *action = new QAction(tr("Properties"));
    menu->addAction(action);
    dpfservice::ProjectInfo info = dpfservice::ProjectInfo::get(item);
    QObject::connect(action, &QAction::triggered, [=]() {
        actionProperties(info, root);
    });

    return menu;
}

void CmakeProjectGenerator::actionTriggered()
{
    using namespace dpfservice;
    QAction *action = qobject_cast<QAction *>(sender());
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
            commandInfo.kitName = CmakeProjectGenerator::toolKitName();
            commandInfo.program = program;
            commandInfo.arguments = args;
            commandInfo.workingDir = workDir;
            builderService->interface.builderCommand({commandInfo}, false);
        }
    }
}

void CmakeProjectGenerator::setRootItemToView(QStandardItem *root)
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

void CmakeProjectGenerator::doBuildCmdExecuteEnd(const BuildCommandInfo &info, int status)
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
        if (info.workingDir == d->configureProjectInfo.workspaceFolder()) {
            reloadItem = val;
            break;
        }
    }
    mutex.unlock();

    if (reloadItem) {
        d->reloadCmakeFileItems.removeOne(reloadItem);   //clean cache
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
    emit projectService->projectConfigureDone(QString());
}

void CmakeProjectGenerator::doCmakeFileNodeChanged(QStandardItem *root, const QPair<QString, QStringList> &files)
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

void CmakeProjectGenerator::actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;

    BuildPropertyWidget *buildWidget = new BuildPropertyWidget(info);
    RunPropertyWidget *runWidget = new RunPropertyWidget(info, item);

    dlg.insertPropertyPanel(tr("Build"), buildWidget);
    dlg.insertPropertyPanel(tr("Run"), runWidget);

    dlg.exec();
}

void CmakeProjectGenerator::recursionRemoveItem(QStandardItem *item)
{
    if (!item)
        return;

    for (int row = 0; row < item->rowCount(); row++) {
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
