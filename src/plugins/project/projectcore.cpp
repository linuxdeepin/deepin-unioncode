// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectcore.h"
#include "transceiver/sendevents.h"
#include "transceiver/projectcorereceiver.h"
#include "mainframe/projectkeeper.h"
#include "mainframe/projecttree.h"
#include "mainframe/recent/recentopenwidget.h"
#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"
#include "services/project/projectservice.h"
#include "services/locator/locatorservice.h"
#include "services/editor/editorservice.h"
#include "locator/allprojectfilelocator.h"
#include "locator/currentprojectlocator.h"
#include "base/abstractwidget.h"

#include <QProcess>
#include <QAction>
#include <QLabel>
#include <QTreeView>

using namespace dpfservice;
void ProjectCore::initialize()
{
    qInfo() << __FUNCTION__;
    // 发布工程服务
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(ProjectService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool ProjectCore::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        if (windowService->addWidgetWorkspace) {
            auto view = new AbstractWidget(ProjectKeeper::instance()->treeView());
            windowService->addWidgetWorkspace(MWCWT_PROJECTS, view, "project");

            DToolButton *focusFile = new DToolButton(ProjectKeeper::instance()->treeView());
            DToolButton *autoFocusSwitcher = new DToolButton(ProjectKeeper::instance()->treeView());
            DToolButton *projectProperty = new DToolButton(ProjectKeeper::instance()->treeView());

            addRecentOpenWidget(windowService);
            addAutoFocusSwitcher(windowService, autoFocusSwitcher, focusFile);
            addProjectProperty(windowService, projectProperty);

            windowService->registerToolBtnToWorkspaceWidget(focusFile, MWCWT_PROJECTS);
			windowService->registerToolBtnToWorkspaceWidget(autoFocusSwitcher, MWCWT_PROJECTS);
            windowService->registerToolBtnToWorkspaceWidget(projectProperty, MWCWT_PROJECTS);
        }
    }

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                     this, &ProjectCore::pluginsStartedMain, Qt::DirectConnection);

    initProject(ctx);
    initLocator(ctx);

    return true;
}

void ProjectCore::addRecentOpenWidget(WindowService *windowService)
{
    RecentOpenWidget *openedWidget = new RecentOpenWidget();
    auto editSrv = dpfGetService(EditorService);
    connect(ProjectProxy::instance(), &ProjectProxy::switchedFile, this, [=](const QString &file){
        openedWidget->setOpenedFiles(editSrv->openedFiles().toVector());
        openedWidget->setListViewSelection(file);
    }, Qt::DirectConnection);
    connect(openedWidget, &RecentOpenWidget::triggered, [=](const QModelIndex &index){
        QFileInfo info(index.data(RecentOpenWidget::RecentOpenedUserRole::FilePathRole).toString());
        if (info.exists() && info.isFile()) {
            editor.openFile(QString(), info.filePath());
        }
    });
    connect(openedWidget, &RecentOpenWidget::closePage, [=](const QModelIndex &index){
        QFileInfo info(index.data(RecentOpenWidget::RecentOpenedUserRole::FilePathRole).toString());
        if (info.exists() && info.isFile()) {
            editor.closeFile(info.filePath());
        }
    });

    auto openFilesWidget = new AbstractWidget(openedWidget);
    windowService->registerWidgetToMode("openFilesWidget", openFilesWidget, CM_EDIT, Position::Left, false, true);
    windowService->setDockHeaderName("openFilesWidget", tr("Opened Files"));
}

void ProjectCore::addAutoFocusSwitcher(WindowService *windowService, DToolButton *autoFocusSwitcher, DToolButton *focusFile)
{
    autoFocusSwitcher->setToolTip(tr("Auto Focus"));
    autoFocusSwitcher->setIcon(QIcon::fromTheme("focus_auto"));
    autoFocusSwitcher->setCheckable(true);
    autoFocusSwitcher->setChecked(true);

    focusFile->setToolTip(tr("Focus File"));
    focusFile->setIcon(QIcon::fromTheme("focus"));

    connect(focusFile, &DToolButton::clicked, this, [](){
        ProjectKeeper::instance()->treeView()->focusCurrentFile();
    }, Qt::DirectConnection);
    focusFile->hide();

    connect(autoFocusSwitcher, &DToolButton::clicked, this, [=](){
        bool state = ProjectKeeper::instance()->treeView()->getAutoFocusState();
        ProjectKeeper::instance()->treeView()->setAutoFocusState(!state);
        if (state) {
            focusFile->show();
        } else {
            focusFile->hide();
        }
    }, Qt::DirectConnection);
}

void ProjectCore::addProjectProperty(WindowService *windowService, DToolButton *projectProperty)
{
    projectProperty->setIcon(QIcon::fromTheme("settings"));
    projectProperty->setToolTip(tr("Open activted project`s property dialog"));
    
    connect(projectProperty, &DToolButton::clicked, this, [=](){
        project.openProjectPropertys(ProjectKeeper::instance()->treeView()->getActiveProjectInfo());
    }, Qt::DirectConnection);
    // todo(zta :temp only supprt cmake project  do other kit later
    connect(ProjectProxy::instance(), &ProjectProxy::projectActivated, this, [=](const ProjectInfo &prjInfo){
        if (prjInfo.kitName() != "cmake")
            projectProperty->setEnabled(false);
        else
            projectProperty->setEnabled(true);
    }, Qt::DirectConnection);
}

void ProjectCore::initLocator(dpf::PluginServiceContext& ctx)
{
    LocatorService *locatorService = ctx.service<LocatorService>(LocatorService::name());
    AllProjectFileLocator *allProjectFileLocator = new AllProjectFileLocator(this);
    CurrentProjectLocator *currentProjectLocator = new CurrentProjectLocator(this);
    locatorService->registerLocator(allProjectFileLocator);
    locatorService->registerLocator(currentProjectLocator);
}

void ProjectCore::initProject(dpf::PluginServiceContext& ctx)
{
    using namespace std::placeholders;
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        ProjectTree *treeView = ProjectKeeper::instance()->treeView();
        if (!projectService->addRootItem) {
            projectService->addRootItem = std::bind(&ProjectTree::appendRootItem, treeView, _1);
        }
        if (!projectService->removeRootItem) {
            projectService->removeRootItem = std::bind(&ProjectTree::removeRootItem, treeView, _1);
        }
        if (!projectService->expandedDepth) {
            projectService->expandedDepth = std::bind(&ProjectTree::expandedProjectDepth, treeView, _1, _2);
        }
        if (!projectService->expandedAll) {
            projectService->expandedAll = std::bind(&ProjectTree::expandedProjectAll, treeView, _1);
        }
        if (!projectService->getAllProjectInfo) {
            projectService->getAllProjectInfo = std::bind(&ProjectTree::getAllProjectInfo, treeView);
        }
        if (!projectService->getProjectInfo) {
            projectService->getProjectInfo = std::bind(&ProjectTree::getProjectInfo, treeView, _1, _2);
        }
        if (!projectService->getActiveProjectInfo) {
            projectService->getActiveProjectInfo = std::bind(&ProjectTree::getActiveProjectInfo, treeView);
        }
        if (!projectService->hasProjectInfo) {
            projectService->hasProjectInfo = std::bind(&ProjectTree::hasProjectInfo, treeView, _1);
        }
        if (!projectService->updateProjectInfo) {
            projectService->updateProjectInfo = std::bind(&ProjectTree::updateProjectInfo, treeView, _1);
        }
        if (!projectService->expandItemByFile) {
            projectService->expandItemByFile = std::bind(&ProjectTree::expandItemByFile, treeView, _1);
        }
    }
}

dpf::Plugin::ShutdownFlag ProjectCore::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

void ProjectCore::pluginsStartedMain()
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (projectService && windowService) {
        QStringList kitNames = projectService->supportGeneratorName<ProjectGenerator>();
        for (auto kitName : kitNames) {
            auto generator = projectService->createGenerator<ProjectGenerator>(kitName);
            if (generator) {
                for (auto lang : generator->supportLanguages()) {
                    auto action = generator->openProjectAction(lang, kitName);
                    if (action)
                        windowService->addOpenProjectAction(lang, new AbstractAction(action));
                }
                QObject::connect(generator, &ProjectGenerator::itemChanged,
                                 ProjectKeeper::instance()->treeView(),
                                 &ProjectTree::itemModified, Qt::UniqueConnection);
            }
        }
    }
}
