// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakeprojectgenerator.h"
#include "cmakeasynparse.h"
#include "cmakeitemkeeper.h"
#include "cmake/project/transceiver/projectcmakereceiver.h"
#include "properties/bulidCfgWidget/buildpropertypage.h"
#include "properties/runCfgWidget/runpropertypage.h"
#include "properties/configWidget/configpropertywidget.h"
#include "properties/kitpage/kitpage.h"
#include "properties/configutil.h"
#include "properties/targetsmanager.h"
#include "services/builder/builderservice.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "common/dialog/propertiesdialog.h"
#include "common/util/eventdefinitions.h"
#include "common/actionmanager/actionmanager.h"
#include "base/abstractaction.h"

#include <QtXml>
#include <QFileIconProvider>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>
#include <QFileSystemWatcher>

using namespace config;
using namespace dpfservice;
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
    QHash<QStandardItem *, QFileSystemWatcher *> projectWatchers;
    QList<QStandardItem *> projectsWaitingUpdate;

    QMap<QStandardItem *, ProjectInfo> cmakeItems;
    bool reConfigure = false;
    QSet<QString> toExpand;
};

CmakeProjectGenerator::CmakeProjectGenerator()
    : d(new CmakeProjectGeneratorPrivate())
{
    // when execute command end can create root Item
    QObject::connect(ProjectCmakeProxy::instance(),
                     &ProjectCmakeProxy::buildExecuteEnd,
                     this, &CmakeProjectGenerator::doBuildCmdExecuteEnd);

    QObject::connect(ProjectCmakeProxy::instance(),
                     &ProjectCmakeProxy::fileDeleted,
                     this, [this](){
        runCMake(this->rootItem, {});
    });

    QObject::connect(ProjectCmakeProxy::instance(),
                     &ProjectCmakeProxy::openProjectPropertys,
                     this, [this](const ProjectInfo &prjInfo){
        auto prjService = dpfGetService(ProjectService);
        if (prjInfo.kitName() == toolKitName())
            actionProperties(prjInfo, prjService->getActiveProjectItem());
    });

    QObject::connect(ProjectCmakeProxy::instance(),
                     &ProjectCmakeProxy::nodeExpanded,
                     this, [this](const QString &filePath){
        d->toExpand.insert(filePath);
    });

    QObject::connect(ProjectCmakeProxy::instance(),
                     &ProjectCmakeProxy::nodeCollapsed,
                     this, [this](const QString &filePath){
        d->toExpand.remove(filePath);
    });

    connect(TargetsManager::instance(), &TargetsManager::initialized, this, &CmakeProjectGenerator::targetInitialized);

    // main thread init watcher class
    CmakeItemKeeper::instance();

    // build cmake file changed notify
    QObject::connect(CmakeItemKeeper::instance(),
                     &CmakeItemKeeper::cmakeFileNodeNotify,
                     this, &CmakeProjectGenerator::runCMake);

    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (!builderService) {
        qCritical() << "Failed, not found service : builderService";
        abort();
    }

    auto mBuild = ActionManager::instance()->actionContainer(M_BUILD);
    // add run cmake menu item.
    QAction *runCMake = new QAction(tr("Run CMake"), this);
    auto cmd = ActionManager::instance()->registerAction(runCMake, "Build.RunCMake");
    mBuild->addAction(cmd);

    QObject::connect(runCMake, &QAction::triggered, this, [this](){
        auto prjService = dpfGetService(ProjectService);
        auto activePrjInfo = prjService->getActiveProjectInfo();
        for (auto item : d->cmakeItems.values()) {
            if (item.isSame(activePrjInfo)) {
                this->runCMake(d->cmakeItems.key(item), {});
                break;
            }
        }
    });

    QObject::connect(config::ConfigUtil::instance(), &config::ConfigUtil::configureDone,
                     [this](const dpfservice::ProjectInfo &info) {
        configure(info);
    });

    // add clear cmake menu item
    QAction *clearCMake = new QAction(tr("Clear CMake"));
    cmd = ActionManager::instance()->registerAction(clearCMake, "Build.ClearCMake");
    mBuild->addAction(cmd);

    QObject::connect(clearCMake, &QAction::triggered, this, [this](){
        auto prjService = dpfGetService(ProjectService);
        auto activePrjInfo = prjService->getActiveProjectInfo();
        for (auto item : d->cmakeItems.values()) {
            if (item.isSame(activePrjInfo))
                this->rootItem = d->cmakeItems.key(item);
        }

        this->clearCMake(this->rootItem);
    });
}

void CmakeProjectGenerator::clearCMake(QStandardItem *root)
{
    auto path = dpfservice::ProjectInfo::get(root).buildFolder();
    auto cmakeFiles = path + "/CMakeFiles";
    auto cmakeCaches = path + "/CMakeCache.txt";
    QFile(cmakeCaches).remove();

    QDir dir(cmakeFiles);
    if (dir.exists()) {
        dir.removeRecursively();
    } else {
        qWarning() << "CMakeFiles directory does not exist.";
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

DWidget *CmakeProjectGenerator::configureWidget(const QString &language,
                                         const QString &workspace)
{
    ProjectGenerator::configureWidget(language, workspace);
    disconnect(this);

    config::ProjectConfigure *param = config::ConfigUtil::instance()->getConfigureParamPointer();
    if (!config::ConfigUtil::instance()->isNeedConfig(workspace, *param)) {
        connect(this, &CmakeProjectGenerator::acceptedConfigure, this, [=]() {
            dpfservice::ProjectInfo info;
            if (config::ConfigUtil::instance()->updateProjectInfo(info, param))
                configure(info);
        });
        return nullptr;
    }

    // show build type config pane.
    ConfigPropertyWidget *configPropertyWidget = new ConfigPropertyWidget(language, workspace);
    connect(this, &CmakeProjectGenerator::acceptedConfigure, configPropertyWidget, [=]() {
        configPropertyWidget->accept();
    });

    return configPropertyWidget;
}

void CmakeProjectGenerator::acceptConfigure()
{
    emit acceptedConfigure();
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

        bool isSuccess = builderService->runbuilderCommand({commandInfo}, false);
        if (isSuccess) {
            ProjectCmakeProxy::instance()->setBuildCommandUuid(commandInfo.uuid);
            // display root item before everything is done.
            // check if run cmake to a existed project by project root path
            auto newRoot = ProjectGenerator::createRootItem(projInfo);
            auto isOpend = ProjectGenerator::isOpenedProject(projInfo.kitName(), projInfo.language(), projInfo.workspaceFolder());
            if (!rootItem || (rootItem->data(Qt::DisplayRole) != newRoot->data(Qt::DisplayRole)) || !isOpend)
                d->reConfigure = false;
            else {
                d->reConfigure = true;
                rootItem->setData(ParsingState::Wait, Parsing_State_Role);
            }

            d->cmakeItems.insert(newRoot, projInfo);
            rootItem = newRoot;
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
    auto fileWatcher = new QFileSystemWatcher(this);
    d->projectWatchers.insert(rootItem, fileWatcher);
    auto thisProject = rootItem;
    d->projectsWaitingUpdate.append(thisProject);

    // asyn free parse, that .project file parse
    QObject::connect(parse, &CmakeAsynParse::parseProjectEnd,
                     [=](CmakeAsynParse::ParseInfo<QStandardItem *> parseInfo) {
                         d->asynItemThreadPolls.remove(parseInfo.result);
                         // active after everything done.
                         project.activeProject(info.kitName(), info.language(), info.workspaceFolder());
                         delete parse;

                         d->projectsWaitingUpdate.removeOne(thisProject);

                         if (!d->reConfigure)
                             return;

                         QMetaObject::invokeMethod(this, [this]() {
                             auto prjService = dpfGetService(ProjectService);
                             prjService->expandItemByFile(d->toExpand.toList());
                         });
                     });

    // asyn execute logic,  that .project file parse
    QtConcurrent::run(d->asynItemThreadPolls[rootItem],
                      parse, &CmakeAsynParse::parseProject,
                      rootItem, info);

    connect(parse, &CmakeAsynParse::directoryCreated, this, [=](const QString &path){
        if (!fileWatcher->directories().contains(path))
            fileWatcher->addPath(path);
    });

    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, [=](const QString &path){
        if (d->projectsWaitingUpdate.contains(thisProject))
            return;

        auto windowService = dpfGetService(WindowService);
        windowService->notify(0, "CMakeProject", tr("Files in project %1 have changed, needs to run cmake to update").arg(thisProject->text()), {});
        d->projectsWaitingUpdate.append(thisProject);
    });

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

    if (d->reloadCmakeFileItems.contains(root))
        d->reloadCmakeFileItems.removeOne(root);

    if (rootItem == root)
        rootItem = nullptr;
    d->cmakeItems.remove(root);

    removeWatcher(root);

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

    createBuildMenu(menu);

    QAction *action = new QAction(tr("Properties"));
    menu->addAction(action);
    dpfservice::ProjectInfo info = dpfservice::ProjectInfo::get(item);
    QObject::connect(action, &QAction::triggered, [=]() {
        actionProperties(info, root);
    });

    return menu;
}

void CmakeProjectGenerator::createDocument(const QStandardItem *item, const QString &filePath)
{
    QString workspace, language;
    auto root = ProjectGenerator::root(const_cast<QStandardItem *>(item));
    if (root) {
        auto rootInfo = dpfservice::ProjectInfo::get(root);
        workspace = rootInfo.workspaceFolder();
        language = rootInfo.language();
    }

    QString fileName = filePath.split("/").last();
    DDialog *dlg = new DDialog();
    dlg->setMessage(tr("File are not automatically added to the "
                       "CmakeList.txt file to the Cmake project. "
                       "Copy the path to the source files to the clipboard?"));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(tr("Copy to Clipboard?"));
    dlg->setIcon(QIcon::fromTheme("dialog-warning"));
    dlg->resize(200, 100);
    dlg->addButton(tr("Ok"), false, DDialog::ButtonType::ButtonRecommend);

    QFile file(filePath);
    if (file.open(QFile::OpenModeFlag::NewOnly)) {
        file.close();
    }

    QObject::connect(dlg, &DDialog::buttonClicked, dlg, [=](){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(fileName);
        dlg->close();
        QString cmakeFilePath;
        QString filePathTmp = filePath;
        QStringList filePathList = filePath.split("/");
        for (int i = filePathList.size() - 1; i >= 0; i--) {
            cmakeFilePath = filePathTmp + QDir::separator() + "CMakeLists.txt";
            if (QFile::exists(cmakeFilePath))
                break;
            filePathTmp.remove(QDir::separator() + filePathList[i]);
        }
        editor.openFile(workspace, cmakeFilePath);
    });
    dlg->exec();
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
            builderService->runbuilderCommand({commandInfo}, false);
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

    if (root) {
        // check if run cmake to a existed project by project root path
        if (!d->reConfigure) {
            projectService->addRootItem(root);
            projectService->expandedDepth(root, 2);
        }

        uiController.doSwitch(MWNA_EDIT);
        uiController.switchWorkspace(MWCWT_PROJECTS);
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

    if (reloadItem && d->reConfigure) {
        projectService->addRootItem(rootItem);
        d->reloadCmakeFileItems.removeOne(reloadItem);   //clean cache

        if (status == 0) {
            projectService->removeRootItem(reloadItem);
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
}

void CmakeProjectGenerator::runCMake(QStandardItem *root, const QPair<QString, QStringList> &files)
{
    Q_UNUSED(files)

    if (d->reloadCmakeFileItems.contains(root))
        return;

    if (rootItem != root)
        rootItem = root;

    // get current project info
    auto proInfo = dpfservice::ProjectInfo::get(root);

    // cache the reload item
    d->reloadCmakeFileItems.append(root);

    removeWatcher(root);

    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();
    ConfigUtil::instance()->updateProjectInfo(proInfo, param);

    // reconfigure project info
    configure(proInfo);
}

void CmakeProjectGenerator::actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;
    dlg.setCurrentTitle(info.currentProgram() + " - " + tr("Project Properties"));

    //update config by current project(not activated project
    ProjectConfigure *projectConfigure = ConfigUtil::instance()->getConfigureParamPointer();
    ConfigUtil::instance()->readConfig(ConfigUtil::instance()->getConfigPath(info.workspaceFolder()), *projectConfigure);

    BuildPropertyPage *buildWidget = new BuildPropertyPage(info);
    RunPropertyPage *runWidget = new RunPropertyPage(info, item);
    KitPage *kitPage = new KitPage(info, item);

    dlg.insertPropertyPanel(tr("Build"), buildWidget);
    dlg.insertPropertyPanel(tr("Run"), runWidget);
    dlg.insertPropertyPanel(tr("Kit"), kitPage);

    connect(buildWidget, &BuildPropertyPage::cacheFileUpdated, this, [=](){
        runCMake(this->rootItem, {});
    });

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
    item = nullptr;
}

void CmakeProjectGenerator::targetInitialized(const QString& workspace)
{
    ProjectConfigure *projectConfigure = ConfigUtil::instance()->getConfigureParamPointer();
    auto tempType = projectConfigure->tempSelType;
    ConfigUtil::instance()->readConfig(ConfigUtil::instance()->getConfigPath(workspace), *projectConfigure);
    projectConfigure->tempSelType = tempType;
    dpfservice::Target activeExecTarget = TargetsManager::instance()->getActivedTargetByTargetType(dpfservice::TargetType::kActiveExecTarget);
    for (auto &buildTypeConfigure : projectConfigure->buildTypeConfigures) {
        if (buildTypeConfigure.type != projectConfigure->tempSelType)
            continue;
        createTargetsRunConfigure(buildTypeConfigure.directory, buildTypeConfigure.runConfigure);

        // update environment.
        for (auto targetRunConfigure : buildTypeConfigure.runConfigure.targetsRunConfigure) {
            if (targetRunConfigure.targetName == activeExecTarget.name) {
                auto projectInfo = dpfGetService(ProjectService)->
                        getProjectInfo(d->configureProjectInfo.kitName(), d->configureProjectInfo.workspaceFolder());
                projectInfo.setRunEnvironment(targetRunConfigure.env.toList());
                dpfGetService(ProjectService)->updateProjectInfo(projectInfo);
            }
        }
    }

    ConfigUtil::instance()->saveConfig(ConfigUtil::instance()->getConfigPath(workspace), *projectConfigure);
}

void CmakeProjectGenerator::createTargetsRunConfigure(const QString &workDirectory, config::RunConfigure &runConfigure)
{
    if (!runConfigure.targetsRunConfigure.isEmpty())
        return;

    QStringList exeTargetList = TargetsManager::instance()->getExeTargetNamesList();
    foreach (auto targetName, exeTargetList) {
        dpfservice::Target target = TargetsManager::instance()->getTargetByName(targetName);

        TargetRunConfigure targetRunConfigure;
        targetRunConfigure.targetName = targetName;
        targetRunConfigure.targetPath = target.output;
        auto targetFile = QFileInfo(targetRunConfigure.targetPath);
        targetRunConfigure.workDirectory = targetFile.exists() ? targetFile.path() : workDirectory;

        runConfigure.targetsRunConfigure.push_back(targetRunConfigure);
    }

    dpfservice::Target target = TargetsManager::instance()->getActivedTargetByTargetType(dpfservice::TargetType::kActiveExecTarget);
    runConfigure.defaultTargetName = target.buildTarget;
}

void CmakeProjectGenerator::createBuildMenu(QMenu *menu)
{
    if (!menu)
        return;

    menu->addSeparator();
    auto addBuildMenu = [&](const QString &actionID){
        auto command = ActionManager::instance()->command(actionID);
        if (command && command->action()) {
            menu->addAction(command->action());
        }
    };
    addBuildMenu("Build.Build");
    addBuildMenu("Build.Rebuild");
    addBuildMenu("Build.Clean");
    addBuildMenu("Build.RunCMake");
    addBuildMenu("Build.ClearCMake");
    menu->addSeparator();
}

void CmakeProjectGenerator::removeWatcher(QStandardItem *root)
{
    auto watcher = d->projectWatchers[root];
    if (watcher)
        delete watcher;
    d->projectWatchers.remove(root);
    if (d->projectsWaitingUpdate.contains(root))
        d->projectsWaitingUpdate.removeOne(root);
}
