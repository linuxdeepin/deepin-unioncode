// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controller.h"
#include "gui/loadingwidget.h"
#include "gui/navigationbar.h"
#include "gui/pluginsui.h"
#include "gui/windowstatusbar.h"
#include "gui/workspacewidget.h"
#include "services/window/windowservice.h"
#include "services/window/windowcontroller.h"
#include "services/project/projectservice.h"
#include "modules/abstractmodule.h"
#include "modules/pluginmanagermodule.h"

#include <DFrame>
#include <DFileDialog>
#include <DTitlebar>
#include <DStackedWidget>
#include <DSearchEdit>

#include <QDebug>
#include <QShortcut>
#include <QDesktopServices>
#include <QMenuBar>
#include <QDesktopWidget>
#include <QScreen>

static Controller *ins { nullptr };

//WN = window name
inline const QString WN_CONTEXTWIDGET = "contextWidget";
inline const QString WN_LOADINGWIDGET = "loadingWidget";
inline const QString WN_WORKSPACE = "workspaceWidget";

// MW = MainWindow
inline constexpr int MW_WIDTH { 1280 };
inline constexpr int MW_HEIGHT { 860 };

inline constexpr int MW_MIN_WIDTH { 1280 };
inline constexpr int MW_MIN_HEIGHT { 600 };
using namespace dpfservice;

DWIDGET_USE_NAMESPACE

struct WidgetInfo
{
    QString name;
    DWidget *widget;
    Position pos;
    bool replace;
    bool isVisible;
};

class ControllerPrivate
{
    MainWindow *mainWindow { nullptr };
    loadingWidget *loadingwidget { nullptr };
    WorkspaceWidget *workspace { nullptr };
    bool showWorkspace { nullptr };

    NavigationBar *navigationBar { nullptr };
    QMap<QString, QAction *> navigationActions;

    DStackedWidget *leftTopToolBar { nullptr };
    DSearchEdit *globalSearchBar { nullptr };
    DWidget *rightTopToolBar { nullptr };
    QMap<QString, DWidget *> topToolBarGroup;

    QMap<QString, DWidget *> contextWidgets;
    QMap<QString, DPushButton *> tabButtons;
    DWidget *contextWidget { nullptr };
    DStackedWidget *stackContextWidget { nullptr };
    DFrame *contextTabBar { nullptr };
    bool contextWidgetAdded { false };

    WindowStatusBar *statusBar { nullptr };

    DMenu *menu { nullptr };

    QMap<QString, View *> viewList;
    QStringList hiddenWidgetList;
    QString currentPlugin { "" };   //navigation - widget
    View *currentView { nullptr };

    QStringList validModeList { CM_EDIT, CM_DEBUG, CM_RECENT };
    QMap<QString, QString> modePluginMap { { CM_EDIT, MWNA_EDIT }, { CM_RECENT, MWNA_RECENT }, { CM_DEBUG, MWNA_DEBUG } };
    QString mode { "" };   //mode: CM_EDIT/CM_DEBUG/CM_RECENT
    QMap<QString, QList<WidgetInfo>> modeInfo;

    QMap<QString, AbstractModule *> modules;

    friend class Controller;
};

Controller *Controller::instance()
{
    if (!ins)
        ins = new Controller;
    return ins;
}

void Controller::registerModule(const QString &moduleName, AbstractModule *module)
{
    Q_ASSERT(module && !moduleName.isEmpty());

    d->modules.insert(moduleName, module);
}

Controller::Controller(QObject *parent)
    : QObject(parent), d(new ControllerPrivate)
{
    initMainWindow();
    initNavigationBar();
    initContextWidget();
    initStatusBar();
    initWorkspaceWidget();
    initTopToolBar();

    registerModule("pluginManagerModule", new PluginManagerModule());
    initModules();

    registerService();
}

void Controller::registerService()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    using namespace std::placeholders;
    if (!windowService->hasView) {
        windowService->hasView = std::bind(&Controller::hasView, this, _1);
    }
    if (!windowService->raiseView) {
        windowService->raiseView = std::bind(&Controller::raiseView, this, _1);
    }
    if (!windowService->raiseMode) {
        windowService->raiseMode = std::bind(&Controller::raiseMode, this, _1);
    }
    if (!windowService->setCurrentPlugin) {
        windowService->setCurrentPlugin = std::bind(&Controller::setCurrentPlugin, this, _1);
    }
    if (!windowService->addWidget) {
        windowService->addWidget = std::bind(&Controller::addWidget, this, _1, _2, _3, _4);
    }
    if (!windowService->replaceWidget) {
        windowService->replaceWidget = std::bind(&Controller::replaceWidget, this, _1, _2, _3);
    }
    if (!windowService->insertWidget) {
        windowService->insertWidget = std::bind(&Controller::insertWidget, this, _1, _2, _3);
    }
    if (!windowService->registerWidgetToMode) {
        windowService->registerWidgetToMode = std::bind(&Controller::registerWidgetToMode, this, _1, _2, _3, _4, _5, _6);
    }
    if (!windowService->addWidgetByOrientation) {
        windowService->addWidgetByOrientation = std::bind(&Controller::addWidgetByOrientation, this, _1, _2, _3, _4, _5);
    }
    if (!windowService->setDockWidgetFeatures) {
        windowService->setDockWidgetFeatures = std::bind(&MainWindow::setDockWidgetFeatures, d->mainWindow, _1, _2);
    }
    if (!windowService->showWidget) {
        //auto showWidgetByName = static_cast<void (MainWindow::*)(const QString &)>(&MainWindow::showWidget);
        windowService->showWidget = std::bind(&Controller::showWidget, this, _1);
    }
    if (!windowService->hideWidget) {
        //auto hideWidgetByName = static_cast<void (MainWindow::*)(const QString &)>(&MainWindow::hideWidget);
        windowService->hideWidget = std::bind(&Controller::hideWidget, this, _1);
    }
    if (!windowService->splitWidgetOrientation) {
        windowService->splitWidgetOrientation = std::bind(&MainWindow::splitWidgetOrientation, d->mainWindow, _1, _2, _3);
    }
    if (!windowService->addNavigationItem) {
        windowService->addNavigationItem = std::bind(&Controller::addNavigationItem, this, _1);
    }
    if (!windowService->addNavigationItemToBottom) {
        windowService->addNavigationItemToBottom = std::bind(&Controller::addNavigationItemToBottom, this, _1);
    }
    if (!windowService->switchWidgetNavigation) {
        windowService->switchWidgetNavigation = std::bind(&Controller::switchWidgetNavigation, this, _1);
    }
    if (!windowService->addContextWidget) {
        windowService->addContextWidget = std::bind(&Controller::addContextWidget, this, _1, _2, _3);
    }
    if (!windowService->hasContextWidget) {
        windowService->hasContextWidget = std::bind(&Controller::hasContextWidget, this, _1);
    }
    if (!windowService->showContextWidget) {
        windowService->showContextWidget = std::bind(&Controller::showContextWidget, this);
    }
    if (!windowService->hideContextWidget) {
        windowService->hideContextWidget = std::bind(&Controller::hideContextWidget, this);
    }
    if (!windowService->switchContextWidget) {
        windowService->switchContextWidget = std::bind(&Controller::switchContextWidget, this, _1);
    }
    if (!windowService->addChildMenu) {
        windowService->addChildMenu = std::bind(&Controller::addChildMenu, this, _1);
    }
    if (!windowService->insertAction) {
        windowService->insertAction = std::bind(&Controller::insertAction, this, _1, _2, _3);
    }
    if (!windowService->addAction) {
        windowService->addAction = std::bind(&Controller::addAction, this, _1, _2);
    }
    if (!windowService->removeActions) {
        windowService->removeActions = std::bind(&Controller::removeActions, this, _1);
    }
    if (!windowService->addOpenProjectAction) {
        windowService->addOpenProjectAction = std::bind(&Controller::addOpenProjectAction, this, _1, _2);
    }
    if (!windowService->addWidgetToTopTool) {
        windowService->addWidgetToTopTool = std::bind(&Controller::addWidgetToTopTool, this, _1, _2, _3, _4);
    }
    if (!windowService->addTopToolItem) {
        windowService->addTopToolItem = std::bind(&Controller::addTopToolItem, this, _1, _2, _3);
    }
    if (!windowService->addTopToolItemToRight) {
        windowService->addTopToolItemToRight = std::bind(&Controller::addTopToolItemToRight, this, _1, _2);
    }
    if (!windowService->showTopToolBar) {
        windowService->showTopToolBar = std::bind(&Controller::showTopToolBar, this, _1);
    }
    if (!windowService->hideTopToolBar) {
        windowService->hideTopToolBar = std::bind(&MainWindow::hideTopTollBar, d->mainWindow);
    }
    if (!windowService->showStatusBar) {
        windowService->showStatusBar = std::bind(&Controller::showStatusBar, this);
    }
    if (!windowService->hideStatusBar) {
        windowService->hideStatusBar = std::bind(&Controller::hideStatusBar, this);
    }
    if (!windowService->addWidgetWorkspace) {
        windowService->addWidgetWorkspace = std::bind(&WorkspaceWidget::addWorkspaceWidget, d->workspace, _1, _2, _3);
    }
}

Controller::~Controller()
{
    if (d->mainWindow)
        delete d->mainWindow;
    foreach (auto view, d->viewList.values())
        delete view;
    foreach (auto module, d->modules.values()) {
        delete module;
        module = nullptr;
    }
    if (d)
        delete d;
}

void Controller::raiseMode(const QString &mode)
{
    if (!d->validModeList.contains(mode)) {
        qWarning() << "mode can only choose CM_RECENT / CM_EDIT / CM_DEBUG";
        return;
    }

    d->mainWindow->hideAllWidget();

    auto widgetInfoList = d->modeInfo[mode];
    foreach (auto widgetInfo, widgetInfoList) {
        if (widgetInfo.replace)
            d->mainWindow->hideWidget(widgetInfo.pos);
        d->mainWindow->showWidget(widgetInfo.name);
        //widget in mainWindow is Dock(widget), show dock and hide widget.
        if (!widgetInfo.isVisible)
            widgetInfo.widget->hide();
    }

    if (mode == CM_RECENT) {
        d->mode = mode;
        return;
    }

    if (mode == CM_EDIT) {
        showTopToolBar(MWTG_EDIT);
        showWorkspace();
    } else if (mode == CM_DEBUG) {
        showTopToolBar(MWTG_DEBUG);
    }

    showContextWidget();
    showStatusBar();

    QString plugin = mode == CM_EDIT ? MWNA_EDIT : MWNA_DEBUG;
    d->currentView = d->viewList[plugin];
    d->mode = mode;
}

void Controller::raiseView(const QString &plugin)
{
    if (!d->viewList.contains(plugin)) {
        qWarning() << "no view of plugin: " << plugin << "try to trigger it";
        d->navigationActions[plugin]->trigger();
        return;
    }

    auto view = d->viewList[plugin];
    if (d->currentView == view)
        return;

    if (!view->mode.isEmpty())
        raiseMode(view->mode);

    bool restoreContextWidget = false;
    foreach (auto hidepos, view->hiddenposList) {
        //if hide centralWidget when contextWidget is visible, contextWidget`s height will resize to window`s height
        if (hidepos == Position::Central && d->contextWidget->isVisible()) {
            d->mainWindow->hideWidget(WN_CONTEXTWIDGET);
            restoreContextWidget = true;
        }
        d->mainWindow->hideWidget(hidepos);
    }

    foreach (auto widgetName, view->widgetList) {
        if (!d->hiddenWidgetList.contains(widgetName))
            d->mainWindow->showWidget(widgetName);
    }

    if (restoreContextWidget)
        d->mainWindow->showWidget(WN_CONTEXTWIDGET);

    d->currentView = view;
}

bool Controller::initView(const QString &widgetName, Position pos, bool replace)
{
    if (d->currentPlugin.isEmpty())
        return false;

    auto view = d->viewList[d->currentPlugin];
    if (!view) {
        view = new View;
        d->viewList.insert(d->currentPlugin, view);
        view->pluginName = d->currentPlugin;
        view->widgetList.append(widgetName);
    } else if (!view->widgetList.contains(widgetName)) {
        view->widgetList.append(widgetName);
    } else {
        return true;
    }

    if (replace) {
        view->hiddenposList.append(pos);
        d->mainWindow->hideWidget(pos);
    }

    return false;
}

bool Controller::hasView(const QString &plugin)
{
    return d->viewList.contains(plugin);
}

void Controller::setCurrentPlugin(const QString &plugin)
{
    d->currentPlugin = plugin;
}

void Controller::addWidget(const QString &name, AbstractWidget *abstractWidget, Position pos, bool replace)
{
    bool viewIsAlreayExisted = initView(name, pos, replace);

    if (viewIsAlreayExisted) {
        raiseView(d->currentPlugin);
        return;
    }

    d->currentView = d->viewList[d->currentPlugin];

    if (abstractWidget) {
        auto widget = static_cast<DWidget *>(abstractWidget->qWidget());
        d->mainWindow->addWidget(name, widget, pos);
    }
}

void Controller::replaceWidget(const QString &name, AbstractWidget *abstractWidget, Position pos)
{
    addWidget(name, abstractWidget, pos, true);
}

void Controller::insertWidget(const QString &name, AbstractWidget *abstractWidget, Position pos)
{
    addWidget(name, abstractWidget, pos, false);
}

void Controller::registerWidgetToMode(const QString &name, AbstractWidget *abstractWidget, const QString &mode, Position pos, bool replace, bool isVisible)
{
    if (!d->validModeList.contains(mode)) {
        qWarning() << "mode can only choose CM_RECENT / CM_EDIT / CM_DEBUG";
        return;
    }
    setCurrentPlugin(d->modePluginMap.value(mode));

    DWidget *qWidget = static_cast<DWidget *>(abstractWidget->qWidget());

    WidgetInfo widgetInfo;
    widgetInfo.name = name;
    widgetInfo.pos = pos;
    widgetInfo.replace = replace;
    widgetInfo.widget = qWidget;
    widgetInfo.isVisible = isVisible;

    initView(name, pos, replace);
    d->mainWindow->addWidget(name, qWidget, pos);
    d->mainWindow->hideWidget(name);

    d->modeInfo[mode].append(widgetInfo);
    setCurrentPlugin("");
}

void Controller::addWidgetByOrientation(const QString &name, AbstractWidget *abstractWidget, Position pos, bool replace, Qt::Orientation orientation)
{
    bool viewIsAlreayExisted = initView(name, pos, replace);

    if (viewIsAlreayExisted) {
        raiseView(d->currentPlugin);
        return;
    }
    if (abstractWidget) {
        auto widget = static_cast<DWidget *>(abstractWidget->qWidget());
        d->mainWindow->addWidget(name, widget, pos, orientation);
    }
    showStatusBar();
}

void Controller::showWidget(const QString &name)
{
    if (!d->hiddenWidgetList.contains(name)) {
        qWarning() << "there is no widget named " << name << "or it`s not hidden";
        return;
    }

    d->mainWindow->showWidget(name);
    d->hiddenWidgetList.removeOne(name);
}

void Controller::hideWidget(const QString &name)
{
    d->hiddenWidgetList.append(name);
    d->mainWindow->hideWidget(name);
}

void Controller::addNavigationItem(AbstractAction *action)
{
    if (!action)
        return;
    auto inputAction = action->qAction();

    d->navigationBar->addNavItem(inputAction);
    d->navigationActions.insert(inputAction->text(), inputAction);
}

void Controller::addNavigationItemToBottom(AbstractAction *action)
{
    if (!action)
        return;
    auto inputAction = action->qAction();

    d->navigationBar->addNavItem(inputAction, NavigationBar::bottom);
    d->navigationActions.insert(inputAction->text(), inputAction);
}

void Controller::switchWidgetNavigation(const QString &navName)
{
    if (navName != d->currentPlugin)
        d->navigationBar->setNavActionChecked(navName, true);

    d->mainWindow->hideTopTollBar();
    setCurrentPlugin(navName);
    if (d->modePluginMap.values().contains(navName))
        raiseMode(d->modePluginMap.key(navName));
    else
        raiseView(navName);

    //send event
    uiController.switchToWidget(navName);
}

void Controller::addContextWidget(const QString &title, AbstractWidget *contextWidget, bool isVisible)
{
    DWidget *qWidget = static_cast<DWidget *>(contextWidget->qWidget());
    if (!qWidget) {
        return;
    }
    d->contextWidgets.insert(title, qWidget);

    d->stackContextWidget->addWidget(qWidget);
    DPushButton *tabBtn = new DPushButton(title);
    tabBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tabBtn->setCheckable(true);
    tabBtn->setFlat(true);
    tabBtn->setFocusPolicy(Qt::NoFocus);

    if (!isVisible)
        tabBtn->hide();

    QHBoxLayout *btnLayout = static_cast<QHBoxLayout *>(d->contextTabBar->layout());
    btnLayout->addWidget(tabBtn);

    connect(tabBtn, &DPushButton::clicked, qWidget, [=] {
        switchContextWidget(title);
    });

    d->tabButtons.insert(title, tabBtn);
}

void Controller::showContextWidget()
{
    if (!d->contextWidgetAdded) {
        d->mainWindow->addWidget(WN_CONTEXTWIDGET, d->contextWidget, Position::Bottom);
        d->contextWidgetAdded = true;
    } else {
        d->mainWindow->showWidget(WN_CONTEXTWIDGET);
    }
    if (!d->currentPlugin.isEmpty() && d->viewList.contains(d->currentPlugin))
        d->viewList[d->currentPlugin]->showContextWidget = true;
}

bool Controller::hasContextWidget(const QString &title)
{
    return d->contextWidgets.contains(title);
}

void Controller::hideContextWidget()
{
    d->mainWindow->hideWidget(WN_CONTEXTWIDGET);
    if (!d->currentPlugin.isEmpty() && d->viewList.contains(d->currentPlugin))
        d->viewList[d->currentPlugin]->showContextWidget = false;
}

void Controller::switchContextWidget(const QString &title)
{
    qInfo() << __FUNCTION__;
    d->stackContextWidget->setCurrentWidget(d->contextWidgets[title]);
    if (d->tabButtons.contains(title))
        d->tabButtons[title]->show();

    for (auto it = d->tabButtons.begin(); it != d->tabButtons.end(); ++it) {
        it.value()->setChecked(false);
        if (it.key() == title)
            it.value()->setChecked(true);
    }
}

void Controller::addChildMenu(AbstractMenu *abstractMenu)
{
    DMenu *inputMenu = abstractMenu->qMenu();
    if (!d->mainWindow || !inputMenu)
        return;

    foreach (AbstractAction *action, abstractMenu->actionList()) {
        if (action && action->hasShortCut())
            registerActionShortCut(action);
    }

    //make the `helper` menu at the last
    for (QAction *action : d->menu->actions()) {
        if (action->text() == MWM_TOOLS) {
            d->menu->insertMenu(action, inputMenu);
            return;
        }
    }

    //add menu to last
    d->menu->addMenu(inputMenu);
}

void Controller::insertAction(const QString &menuName, const QString &beforActionName, AbstractAction *action)
{
    if (!action)
        return;

    QAction *inputAction = action->qAction();
    if (!inputAction)
        return;

    if (action->hasShortCut())
        registerActionShortCut(action);

    for (QAction *qAction : d->mainWindow->menuBar()->actions()) {
        if (qAction->text() == menuName) {
            for (auto childAction : qAction->menu()->actions()) {
                if (childAction->text() == beforActionName) {
                    qAction->menu()->insertAction(childAction, inputAction);
                    break;
                } else if (qAction->text() == MWM_FILE
                           && childAction->text() == MWMFA_QUIT) {
                    qAction->menu()->insertAction(qAction, inputAction);
                    break;
                }
            }
        }
    }
}

void Controller::addAction(const QString &menuName, AbstractAction *action)
{
    QAction *inputAction = action->qAction();
    if (!inputAction)
        return;

    if (action->hasShortCut())
        registerActionShortCut(action);

    //防止与edit和debug界面的topToolBar快捷键冲突
    if (menuName != MWM_DEBUG && menuName != MWM_BUILD)
        addMenuShortCut(inputAction);

    if (menuName == MWMFA_NEW_FILE_OR_PROJECT) {
        for (QAction *qAction : d->menu->actions()) {
            if (qAction->text() == MWM_BUILD) {
                d->menu->insertAction(qAction, inputAction);
                d->menu->insertSeparator(qAction);
                return;
            }
        }
    }

    for (QAction *qAction : d->menu->actions()) {
        if (qAction->text() == menuName) {
            if (qAction->text() == MWM_FILE) {
                auto endAction = *(qAction->menu()->actions().rbegin());
                if (endAction->text() == MWMFA_QUIT) {
                    return qAction->menu()->insertAction(endAction, inputAction);
                }
            }
            qAction->menu()->addAction(inputAction);
        }
    }

    if (!inputAction->parent())
        inputAction->setParent(this);
}

void Controller::removeActions(const QString &menuName)
{
    for (QAction *qAction : d->mainWindow->menuBar()->actions()) {
        if (qAction->text() == menuName) {
            foreach (QAction *action, qAction->menu()->actions()) {
                qAction->menu()->removeAction(action);
            }
            break;
        }
    }
}

void Controller::addOpenProjectAction(const QString &name, AbstractAction *action)
{
    if (!action || !action->qAction())
        return;

    if (action->hasShortCut())
        registerActionShortCut(action);

    QAction *inputAction = action->qAction();

    foreach (QAction *action, d->menu->actions()) {
        if (action->text() == MWMFA_OPEN_PROJECT) {
            for (auto langAction : action->menu()->menuAction()->menu()->actions()) {
                if (name == langAction->text()) {
                    langAction->menu()->addAction(inputAction);
                    return;
                }
            }
            auto langMenu = new DMenu(name);
            action->menu()->addMenu(langMenu);
            langMenu->addAction(inputAction);
            return;
        }
    }
}

void Controller::addWidgetToTopTool(AbstractWidget *abstractWidget, const QString &group, bool addSeparator, bool addToLeft)
{
    if (!abstractWidget)
        return;
    auto widget = static_cast<DWidget *>(abstractWidget->qWidget());
    if(!widget)
        return;

    QHBoxLayout *hlayout { nullptr };
    DWidget *toolBar { nullptr };

    if (!addToLeft) {
        hlayout = qobject_cast<QHBoxLayout *>(d->rightTopToolBar->layout());
    } else if (d->topToolBarGroup.contains(group)) {
        toolBar = d->topToolBarGroup[group];
        hlayout = qobject_cast<QHBoxLayout *>(toolBar->layout());
    } else {
        toolBar = new DWidget(d->leftTopToolBar);
        hlayout = new QHBoxLayout(toolBar);
        hlayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        hlayout->setSpacing(10);
        hlayout->setContentsMargins(0, 0, 0, 0);
        d->topToolBarGroup.insert(group, toolBar);
        d->leftTopToolBar->addWidget(toolBar);
    }

    if (addSeparator) {
        DVerticalLine *line = new DVerticalLine(d->mainWindow);
        line->setFixedHeight(20);
        line->setFixedWidth(1);
        hlayout->addWidget(line);
    }

    hlayout->addWidget(widget);
}

void Controller::addTopToolItem(AbstractAction *action, const QString &group, bool addSeparator)
{
    if (!action || !action->qAction())
        return;

    if (action->hasShortCut())
        registerActionShortCut(action);

    auto iconBtn = createIconButton(action->qAction());
    addWidgetToTopTool(new AbstractWidget(iconBtn), group, addSeparator, true);
}

void Controller::addTopToolItemToRight(AbstractAction *action, bool addSeparator)
{
    if (!action || !action->qAction())
        return;

    if (action->hasShortCut())
        registerActionShortCut(action);

    auto iconBtn = createIconButton(action->qAction());
    addWidgetToTopTool(new AbstractWidget(iconBtn), "", addSeparator, false);
}

void Controller::showTopToolBar(const QString &group)
{
    if (!d->topToolBarGroup.contains(group))
        return;

    d->mainWindow->showTopToolBar();
    d->leftTopToolBar->setCurrentWidget(d->topToolBarGroup[group]);
}

void Controller::openFileDialog()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = DFileDialog::getOpenFileName(nullptr, tr("Open Document"), dir);
    if (filePath.isEmpty() && !QFileInfo(filePath).exists())
        return;
    recent.saveOpenedFile(filePath);
    editor.openFile(filePath);
}

void Controller::loading()
{
    d->loadingwidget = new loadingWidget(d->mainWindow);
    d->mainWindow->addWidget(WN_LOADINGWIDGET, d->loadingwidget);

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                     this, [=]() {
                         d->mainWindow->removeWidget(WN_LOADINGWIDGET);

                         d->navigationBar->show();
                         d->mainWindow->setToolbar(Qt::ToolBarArea::LeftToolBarArea, d->navigationBar);
                     });
}

void Controller::initMainWindow()
{
    qInfo() << __FUNCTION__;
    if (!d->mainWindow) {
        d->mainWindow = new MainWindow;
        d->mainWindow->setMinimumSize(MW_MIN_WIDTH, MW_MIN_HEIGHT);
        d->mainWindow->resize(MW_WIDTH, MW_HEIGHT);

        initMenu();

        if (CommandParser::instance().getModel() != CommandParser::CommandLine) {
            d->mainWindow->showMaximized();
            loading();
        }

        int currentScreenIndex = qApp->desktop()->screenNumber(d->mainWindow);
        QList<QScreen *> screenList = QGuiApplication::screens();

        if (currentScreenIndex < screenList.count()) {
            QRect screenRect = screenList[currentScreenIndex]->geometry();
            int screenWidth = screenRect.width();
            int screenHeight = screenRect.height();
            d->mainWindow->move((screenWidth - d->mainWindow->width()) / 2, (screenHeight - d->mainWindow->height()) / 2);
        }
    }
}

void Controller::initNavigationBar()
{
    qInfo() << __FUNCTION__;
    if (d->navigationBar)
        return;
    d->navigationBar = new NavigationBar(d->mainWindow);
    d->navigationBar->hide();
}

void Controller::initMenu()
{
    qInfo() << __FUNCTION__;
    if (!d->mainWindow)
        return;
    if (!d->menu)
        d->menu = new DMenu(d->mainWindow->titlebar());

    createFileActions();
    createBuildActions();
    createDebugActions();

    d->menu->addSeparator();

    createHelpActions();
    createToolsActions();

    d->mainWindow->titlebar()->setMenu(d->menu);
}

void Controller::createHelpActions()
{
    auto helpMenu = new DMenu(MWM_HELP, d->menu);
    d->menu->addMenu(helpMenu);

    QAction *actionReportBug = new QAction(MWM_REPORT_BUG, helpMenu);
    ActionManager::getInstance()->registerAction(actionReportBug, "Help.Report.Bug",
                                                 MWM_REPORT_BUG, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_R));
    addMenuShortCut(actionReportBug);
    helpMenu->addAction(actionReportBug);

    QAction *actionHelpDoc = new QAction(MWM_HELP_DOCUMENTS, helpMenu);
    ActionManager::getInstance()->registerAction(actionHelpDoc, "Help.Help.Documents",
                                                 MWM_HELP_DOCUMENTS, QKeySequence());
    helpMenu->addAction(actionHelpDoc);
    addMenuShortCut(actionHelpDoc);

    helpMenu->addSeparator();



    QAction::connect(actionReportBug, &QAction::triggered, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://github.com/linuxdeepin/deepin-unioncode/issues"));
    });
    QAction::connect(actionHelpDoc, &QAction::triggered, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://ecology.chinauos.com/adaptidentification/doc_new/#document2?dirid=656d40a9bd766615b0b02e5e"));
    });
}

void Controller::createToolsActions()
{
    auto toolsMenu = new DMenu(MWM_TOOLS);
    d->menu->addMenu(toolsMenu);
}

void Controller::createDebugActions()
{
    auto *debugMenu = new DMenu(MWM_DEBUG);
    d->menu->addMenu(debugMenu);
}

void Controller::createBuildActions()
{
    auto *buildMenu = new DMenu(MWM_BUILD);
    d->menu->addMenu(buildMenu);
}

void Controller::createFileActions()
{
    QAction *actionOpenFile = new QAction(MWMFA_OPEN_FILE);
    ActionManager::getInstance()->registerAction(actionOpenFile, "File.Open.File",
                                                 MWMFA_OPEN_FILE, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_O));

    QAction::connect(actionOpenFile, &QAction::triggered, this, &Controller::openFileDialog);
    d->menu->addAction(actionOpenFile);
    addMenuShortCut(actionOpenFile);

    DMenu *menuOpenProject = new DMenu(MWMFA_OPEN_PROJECT);
    d->menu->addMenu(menuOpenProject);
}

void Controller::initContextWidget()
{
    if (!d->stackContextWidget)
        d->stackContextWidget = new DStackedWidget(d->mainWindow);
    if (!d->contextTabBar)
        d->contextTabBar = new DFrame(d->mainWindow);
    if (!d->contextWidget)
        d->contextWidget = new DWidget(d->mainWindow);

    DStyle::setFrameRadius(d->contextTabBar, 0);
    d->contextTabBar->setLineWidth(0);
    QHBoxLayout *contextTabLayout = new QHBoxLayout(d->contextTabBar);
    contextTabLayout->setAlignment(Qt::AlignLeft);

    QVBoxLayout *contextVLayout = new QVBoxLayout();
    contextVLayout->setContentsMargins(0, 0, 0, 0);
    contextVLayout->setSpacing(0);
    contextVLayout->addWidget(d->contextTabBar);
    contextVLayout->addWidget(new DHorizontalLine);
    contextVLayout->addWidget(d->stackContextWidget);
    d->contextWidget->setLayout(contextVLayout);

    //add contextWidget after add centralWidget or it`s height is incorrect
    //d->mainWindow->addWidget(WN_CONTEXTWIDGET, d->contextWidget, Position::Bottom);
}

void Controller::initStatusBar()
{
    if (d->statusBar)
        return;
    d->statusBar = new WindowStatusBar(d->mainWindow);
    d->statusBar->hide();
    d->mainWindow->setStatusBar(d->statusBar);
}

void Controller::initWorkspaceWidget()
{
    if (d->workspace)
        return;
    d->workspace = new WorkspaceWidget(d->mainWindow);
}

void Controller::initTopToolBar()
{
    d->leftTopToolBar = new DStackedWidget(d->mainWindow);
    d->globalSearchBar = new DSearchEdit(d->mainWindow);
    d->rightTopToolBar = new DWidget(d->mainWindow);

    QHBoxLayout *rtLayout = new QHBoxLayout(d->rightTopToolBar);
    rtLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rtLayout->setContentsMargins(0, 0, 0, 0);
    rtLayout->setSpacing(10);

    d->mainWindow->setLeftTopToolWidget(d->leftTopToolBar);
    d->mainWindow->setMiddleTopToolWidget(d->globalSearchBar);
    d->mainWindow->setRightTopToolWidget(d->rightTopToolBar);

    d->mainWindow->hideTopTollBar();
}

void Controller::initModules()
{
    for (auto module : d->modules.values()) {
        module->initialize(this);
    }
}

void Controller::addMenuShortCut(QAction *action, QKeySequence keySequence)
{
    QKeySequence key = keySequence;
    if (keySequence.isEmpty())
        key = action->shortcut();

    QShortcut *shortCut = new QShortcut(key, d->mainWindow);
    connect(action, &QAction::changed, this, [=]() {
        if (action->shortcut() != shortCut->key())
            shortCut->setKey(action->shortcut());
    });
    connect(shortCut, &QShortcut::activated, action, [=] {
        action->trigger();
    });
}

void Controller::showStatusBar()
{
    if (d->statusBar)
        d->statusBar->show();
}

void Controller::hideStatusBar()
{
    if (d->statusBar)
        d->statusBar->hide();
}

void Controller::switchWorkspace(const QString &titleName)
{
    if (d->workspace)
        d->workspace->switchWidgetWorkspace(titleName);
}

void Controller::registerActionShortCut(AbstractAction *action)
{
    auto qAction = action->qAction();
    if (!qAction)
        return;
    ActionManager::getInstance()->registerAction(qAction, action->id(), action->description(), action->keySequence());
}

void Controller::showWorkspace()
{
    if (d->showWorkspace != true) {
        d->mainWindow->addWidget(WN_WORKSPACE, d->workspace, Position::Left);
        d->mainWindow->resizeDock(WN_WORKSPACE, QSize(300, 300));
        d->showWorkspace = true;
    }

    d->mainWindow->showWidget(WN_WORKSPACE);
}

DIconButton *Controller::createIconButton(QAction *action)
{
    DIconButton *iconBtn = new DIconButton(d->mainWindow);
    iconBtn->setFocusPolicy(Qt::NoFocus);
    iconBtn->setEnabled(action->isEnabled());
    iconBtn->setIcon(action->icon());
    iconBtn->setFixedSize(QSize(36, 36));
    iconBtn->setIconSize(QSize(15, 15));

    QString toolTipStr = action->text() + " " + action->shortcut().toString();
    iconBtn->setToolTip(toolTipStr);
    iconBtn->setShortcut(action->shortcut());

    connect(iconBtn, &DIconButton::clicked, action, &QAction::triggered);
    connect(action, &QAction::changed, iconBtn, [=] {
        if (action->shortcut() != iconBtn->shortcut()) {
            QString toolTipStr = action->text() + " " + action->shortcut().toString();
            iconBtn->setToolTip(toolTipStr);
            iconBtn->setShortcut(action->shortcut());
        }

        iconBtn->setEnabled(action->isEnabled());
    });

    return iconBtn;
}
