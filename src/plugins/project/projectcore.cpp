// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectcore.h"
#include "transceiver/sendevents.h"
#include "transceiver/projectcorereceiver.h"
#include "mainframe/projectkeeper.h"
#include "mainframe/projecttree.h"
#include "mainframe/recent/recentopenwidget.h"
#include "common/common.h"
#include "common/util/macroexpander.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"
#include "services/project/projectservice.h"
#include "services/project/directorygenerator.h"
#include "services/locator/locatorservice.h"
#include "services/editor/editorservice.h"
#include "locator/allprojectfilelocator.h"
#include "locator/currentprojectlocator.h"
#include "base/abstractwidget.h"

#include <DDialog>
#include <DStackedWidget>

#include <QProcess>
#include <QAction>
#include <QLabel>
#include <QTreeView>
#include <QFileDialog>
#include <QWidget>
#include <QComboBox>

using namespace dpfservice;
static const QString openFilesWidgetName = "openFilesWidget";
static bool openFileWidgetInited = false;

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

    connect(ProjectProxy::instance(), &ProjectProxy::openProject, this, [=](const QString &directory) {
        confirmProjectKit(directory);
    },
            Qt::DirectConnection);

    initProject(ctx);
    initLocator(ctx);
    registerVariables();

    return true;
}

void ProjectCore::addRecentOpenWidget(WindowService *windowService)
{
    RecentOpenWidget *openedWidget = new RecentOpenWidget();
    auto editSrv = dpfGetService(EditorService);
    connect(ProjectProxy::instance(), &ProjectProxy::switchedFile, this, [=](const QString &file) {
        openedWidget->setOpenedFiles(editSrv->openedFiles().toVector());
        openedWidget->setListViewSelection(file);
    },
            Qt::DirectConnection);
    connect(openedWidget, &RecentOpenWidget::triggered, [=](const QModelIndex &index) {
        QFileInfo info(index.data(RecentOpenWidget::RecentOpenedUserRole::FilePathRole).toString());
        if (info.exists() && info.isFile()) {
            editor.openFile(QString(), info.filePath());
        }
    });
    connect(openedWidget, &RecentOpenWidget::closePage, [=](const QModelIndex &index) {
        QFileInfo info(index.data(RecentOpenWidget::RecentOpenedUserRole::FilePathRole).toString());
        if (info.exists() && info.isFile()) {
            editor.closeFile(info.filePath());
        }
    });
    connect(ProjectProxy::instance(), &ProjectProxy::modeRaised, this, [=](const QString &mode) {
        if (mode != CM_EDIT || openFileWidgetInited)
            return;
        initOpenFilesWidget(windowService);
    },
            Qt::DirectConnection);
    auto openFilesWidget = new AbstractWidget(openedWidget);
    openFilesWidget->setDisplayIcon(QIcon::fromTheme("opened_files"));
    windowService->registerWidgetToMode(openFilesWidgetName, openFilesWidget, CM_EDIT, Position::Left, false, true);
    windowService->setDockHeaderName(openFilesWidgetName, tr("Opened Files"));
}

void ProjectCore::addAutoFocusSwitcher(WindowService *windowService, DToolButton *autoFocusSwitcher, DToolButton *focusFile)
{
    autoFocusSwitcher->setToolTip(tr("Auto Focus"));
    autoFocusSwitcher->setIcon(QIcon::fromTheme("focus_auto"));
    autoFocusSwitcher->setCheckable(true);
    autoFocusSwitcher->setChecked(true);

    focusFile->setToolTip(tr("Focus File"));
    focusFile->setIcon(QIcon::fromTheme("focus"));

    connect(focusFile, &DToolButton::clicked, this, []() {
        ProjectKeeper::instance()->treeView()->focusCurrentFile();
    },
            Qt::DirectConnection);
    focusFile->hide();

    connect(autoFocusSwitcher, &DToolButton::clicked, this, [=]() {
        bool state = ProjectKeeper::instance()->treeView()->getAutoFocusState();
        ProjectKeeper::instance()->treeView()->setAutoFocusState(!state);
        if (state) {
            focusFile->show();
        } else {
            focusFile->hide();
        }
    },
            Qt::DirectConnection);
}

void ProjectCore::addProjectProperty(WindowService *windowService, DToolButton *projectProperty)
{
    projectProperty->setIcon(QIcon::fromTheme("settings"));
    projectProperty->setToolTip(tr("Open activted project`s property dialog"));

    connect(projectProperty, &DToolButton::clicked, this, [=]() {
        project.openProjectPropertys(ProjectKeeper::instance()->treeView()->getActiveProjectInfo());
    },
            Qt::DirectConnection);
    // todo(zta :temp only supprt cmake project  do other kit later
    connect(ProjectProxy::instance(), &ProjectProxy::projectActivated, this, [=](const ProjectInfo &prjInfo) {
        if (prjInfo.kitName() != "cmake")
            projectProperty->setEnabled(false);
        else
            projectProperty->setEnabled(true);
    },
            Qt::DirectConnection);
}

void ProjectCore::initLocator(dpf::PluginServiceContext &ctx)
{
    LocatorService *locatorService = ctx.service<LocatorService>(LocatorService::name());
    AllProjectFileLocator *allProjectFileLocator = new AllProjectFileLocator(this);
    CurrentProjectLocator *currentProjectLocator = new CurrentProjectLocator(this);
    locatorService->registerLocator(allProjectFileLocator);
    locatorService->registerLocator(currentProjectLocator);
}

void ProjectCore::initProject(dpf::PluginServiceContext &ctx)
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
        if (!projectService->openProject) {
            projectService->openProject = std::bind(&ProjectCore::openProject, this);
        }
        if (!projectService->getActiveProjectItem) {
            projectService->getActiveProjectItem = std::bind(&ProjectTree::getActiveProjectItem, treeView);
        }
        if (!projectService->restoreExpandState) {
            projectService->restoreExpandState = std::bind(&ProjectTree::restoreExpandState, treeView, _1);
        }
    }
}

void ProjectCore::registerVariables()
{
    globalMacroExpander()->registerFileVariables("CurrentProject",
                                                 tr("Current project"),
                                                 [] {
                                                     auto info = ProjectKeeper::instance()->treeView()->getActiveProjectInfo();
                                                     return QFileInfo(info.workspaceFolder());
                                                 });
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
        QString errorString;
        if (!projectService->implGenerator<DirectoryGenerator>(DirectoryGenerator::toolKitName(), &errorString))
            qWarning() << errorString;
        QStringList kitNames = projectService->supportGeneratorName<ProjectGenerator>();
        for (auto kitName : kitNames) {
            auto generator = projectService->createGenerator<ProjectGenerator>(kitName);
            if (generator) {
                QObject::connect(generator, &ProjectGenerator::itemChanged,
                                 ProjectKeeper::instance()->treeView(),
                                 &ProjectTree::itemModified, Qt::UniqueConnection);
            }
        }
    }
}

void ProjectCore::initOpenFilesWidget(dpfservice::WindowService *windowService)
{
    // adjust position
    windowService->splitWidgetOrientation(WN_WORKSPACE, openFilesWidgetName, Qt::Vertical);

    QStringList allLeftDocks = windowService->getCurrentDockName(Position::Left);
    auto dockCount = allLeftDocks.size();

    QStringList docks;
    // Set the height of the widget to 25% of the total height.
    QList<int> sizes;
    for (const auto &dock : allLeftDocks) {
        if (dock == WN_WORKSPACE)
            sizes << 100 - (dockCount - 1) * 15;
        else
            sizes << 15;
        docks << dock;
    }

    windowService->resizeDocks(docks, sizes, Qt::Vertical);

    openFileWidgetInited = true;
}

void ProjectCore::openProject()
{
    QString iniPath = CustomPaths::user(CustomPaths::Flags::Configures)
            + QDir::separator() + QString("project_record.support");
    QSettings setting(iniPath, QSettings::IniFormat);
    QString lastPath = setting.value("recent_open_project").toString();

    QFileDialog fileDialog;
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.setOption(QFileDialog::DontResolveSymlinks);
    fileDialog.setWindowTitle(QFileDialog::tr("Open Project Directory"));
    fileDialog.setDirectory(lastPath);
    fileDialog.setWindowFlags(fileDialog.windowFlags() | Qt::WindowStaysOnTopHint);
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString projectPath = fileDialog.selectedUrls().first().path();
    confirmProjectKit(projectPath);
    setting.setValue("recent_open_project", projectPath);   // save open history
}

void ProjectCore::confirmProjectKit(const QString &path)
{
    DDialog dialog;
    dialog.setWindowTitle(tr("Config"));
    dialog.setIcon(QIcon::fromTheme("ide"));
    auto widget = new QWidget;
    auto layout = new QHBoxLayout(widget);
    layout->setAlignment(Qt::AlignLeft);

    QLabel *label = new QLabel(tr("Project Type:"), widget);
    label->setFixedWidth(100);
    QComboBox *cbBox = new QComboBox(widget);
    layout->addWidget(label);
    layout->addWidget(cbBox);
    dialog.addContent(widget);
    dialog.addButton(tr("Cancel"));
    dialog.addButton(tr("Confirm"), true, DDialog::ButtonRecommend);
    auto originalSize = dialog.size();

    auto projectService = dpfGetService(ProjectService);
    auto allKits = projectService->supportGeneratorName<ProjectGenerator>();
    cbBox->addItems(allKits);
    DStackedWidget *configureWidget = new DStackedWidget;
    dialog.addContent(configureWidget);

    // check defualt kit by supportNames
    bool hasDefault = false;
    for (auto kit : allKits) {
        if (hasDefault)
            break;
        auto generator = projectService->createGenerator<ProjectGenerator>(kit);
        QStringList fileNames = generator->supportFileNames();
        if (fileNames.isEmpty())
            continue;
        for (auto filename : fileNames) {
            if (QDir(path).exists(filename)) {
                cbBox->setCurrentText(kit);
                auto widget = generator->configureWidget(generator->supportLanguages().first(), path);
                if (widget)
                    configureWidget->addWidget(widget);
                hasDefault = true;
                break;
            }
        }
    }

    if (!hasDefault) {
        // Apply language-specific processing by suffix
        QString kit = DirectoryGenerator::toolKitName();
        for (auto fileInfo : QDir(path).entryInfoList()) {
            if (fileInfo.suffix() == "py")
                kit = "python";
            else if (fileInfo.suffix() == "js")
                kit = "javascript";
            else
                continue;

            break;
        }
        auto generator = projectService->createGenerator<ProjectGenerator>(kit);
        if (!generator) {
            kit = DirectoryGenerator::toolKitName();
            generator = projectService->createGenerator<ProjectGenerator>(DirectoryGenerator::toolKitName());
        }
        cbBox->setCurrentText(kit);
        auto widget = generator->configureWidget(generator->supportLanguages().first(), path);
        if (widget)
            configureWidget->addWidget(widget);
    }

    // select kit
    connect(cbBox, &QComboBox::currentTextChanged, this, [=, &dialog, &originalSize](const QString &text) {
        auto generator = projectService->createGenerator<ProjectGenerator>(text);
        auto currentWidget = configureWidget->currentWidget();
        if (currentWidget) {
            configureWidget->removeWidget(currentWidget);
            delete currentWidget;
        }
        auto widget = generator->configureWidget(generator->supportLanguages().first(), path);
        if (!widget) {
            QTimer::singleShot(20, &dialog, [&]() { dialog.resize(originalSize); });
        } else {
            configureWidget->addWidget(widget);
        }
    },
            Qt::DirectConnection);

    if (dialog.exec() == DDialog::Accepted) {
        auto kit = cbBox->currentText();
        auto generator = projectService->createGenerator<ProjectGenerator>(kit);
        if (generator->canOpenProject(kit, generator->supportLanguages().first(), path))
            generator->acceptConfigure();
    }
}
