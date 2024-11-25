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
#include "modules/documentfindmodule.h"
#include "modules/contextmodule.h"
#include "modules/notificationmodule.h"
#include "modules/dependencemodule.h"
#include "modules/sessionmanagermodule.h"
#include "locator/locatormanager.h"
#include "find/placeholdermanager.h"

#include "common/util/utils.h"
#include "common/actionmanager/actioncontainer.h"

#include <DFrame>
#include <DFileDialog>
#include <DTitlebar>
#include <DStackedWidget>
#include <DSearchEdit>
#include <DFontSizeManager>

#include <QDebug>
#include <QShortcut>
#include <QDesktopServices>
#include <QMenuBar>
#include <QDesktopWidget>
#include <QScreen>
#include <QApplication>

static Controller *ins { nullptr };

inline const QString WN_LOADINGWIDGET = "loadingWidget";

// MW = MainWindow
inline constexpr int MW_WIDTH { 1280 };
inline constexpr int MW_HEIGHT { 860 };

inline constexpr int MW_MIN_WIDTH { 1080 };
inline constexpr int MW_MIN_HEIGHT { 600 };
using namespace dpfservice;

DWIDGET_USE_NAMESPACE

struct WidgetInfo
{
    QString name;
    QDockWidget *dockWidget { nullptr };
    QString headerName;
    bool showHeader { true };
    QList<QAction *> headerList;
    QList<QWidget *> headerWidget;   // set button to header after create dock
    QIcon icon;

    Position defaultPos;   // set position after create dock
    bool replace { false };   // hide current position`s dock before show
    bool defaultVisible { true };
    bool created { false };   // has already create dock
    bool hiddenByManual { false };
    
    std::function<AbstractWidget*()> createDWidgetFunc;

    DWidget* getDWidget()
    {
        if (!widget) {
            if (createDWidgetFunc) {
                auto abstractWidget = createDWidgetFunc(); 
                icon = abstractWidget->getDisplayIcon();
                widget = static_cast<DWidget *>(abstractWidget->qWidget());
                Q_ASSERT(widget);
                if (!widget->parent())
                    widget->setParent(Controller::instance()->mainWindow());
            }
        }
        return widget;
    }
    
    void setWidget(DWidget *widget)
    {
        this->widget = widget;
    }

    bool operator==(const WidgetInfo &info)
    {
        if (name == info.name && widget == info.widget)
            return true;
        return false;
    };

private:    
    DWidget *widget { nullptr };
};

class DocksManagerButton : public DToolButton
{
public:
    explicit DocksManagerButton(QWidget *parent, Controller *con)
        : DToolButton(parent), controller(con) { setMouseTracking(true); }

protected:
    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (controller)
            controller->showCurrentDocksManager();
    }

private:
    Controller *controller { nullptr };
};

class ControllerPrivate
{
    MainWindow *mainWindow { nullptr };
    loadingWidget *loadingwidget { nullptr };
    WorkspaceWidget *workspace { nullptr }; // left default dock widget
    WorkspaceWidget *rightspace { nullptr }; // right default dock widget

    DWidget *navigationToolBar { nullptr };
    NavigationBar *navigationBar { nullptr };
    QMap<QString, QAction *> navigationActions;
    QMap<QString, QAction *> widgetBindToNavigation;
    DocksManagerButton *docksManager { nullptr };

    DWidget *leftTopToolBar { nullptr };
    DSearchEdit *locatorBar { nullptr };
    DWidget *rightTopToolBar { nullptr };
    QMap<QAction *, DToolButton *> topToolBtn;
    QMap<QString, DToolButton *> dockButtons;

    QMap<QString, DWidget *> contextWidgets;
    QMap<QString, DPushButton *> tabButtons;
    DWidget *contextWidget { nullptr };
    DStackedWidget *stackContextWidget { nullptr };
    DFrame *contextTabBar { nullptr };
    QHBoxLayout *contextButtonLayout { nullptr };

    WindowStatusBar *statusBar { nullptr };
    DToolButton *showContextBtn { nullptr };
    DToolButton *showRightspaceBtn { nullptr };

    QStringList validModeList { CM_EDIT, CM_DEBUG, CM_RECENT };
    QMap<QString, QString> modePluginMap { { CM_EDIT, MWNA_EDIT }, { CM_RECENT, MWNA_RECENT }, { CM_DEBUG, MWNA_DEBUG } };
    QString mode { "" };   // mode: CM_EDIT/CM_DEBUG/CM_RECENT
    QMap<QString, QStringList> modeInfo;
    QString currentNavigation { "" };

    QMap<QString, AbstractModule *> modules;
    QMap<QString, WidgetInfo> allWidgets;
    QList<QString> currentDocks;

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

MainWindow *Controller::mainWindow() const
{
    return d->mainWindow;
}

Controller::Controller(QObject *parent)
    : QObject(parent), d(new ControllerPrivate)
{
    initMainWindow();
    initNavigationBar();
    initStatusBar();
    initRightspaceWidget();
    initContextWidget();
    initWorkspaceWidget();
    initTopToolBar();
    initDocksManager();
    registerService();

    registerModule("pluginManagerModule", new PluginManagerModule());
    registerModule("docFindModule", new DocumentFindModule());
    registerModule("contextModule", new ContextModule());
    registerModule("notifyModule", new NotificationModule());
    registerModule("dependenceModule", new DependenceModule());
    registerModule("sessionManagerModule", new SessionManagerModule());
    initModules();
}

void Controller::registerService()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    using namespace std::placeholders;
    if (!windowService->raiseMode) {
        windowService->raiseMode = std::bind(&Controller::raiseMode, this, _1);
    }
    if (!windowService->replaceWidget) {
        windowService->replaceWidget = std::bind(&Controller::replaceWidget, this, _1, _2);
    }
    if (!windowService->insertWidget) {
        windowService->insertWidget = std::bind(&Controller::insertWidget, this, _1, _2, _3);
    }
    if (!windowService->hideWidget) {
        windowService->hideWidget = std::bind(&Controller::hideWidget, this, _1);
    }
    if (!windowService->registerWidgetToMode) {
        windowService->registerWidgetToMode = std::bind(&Controller::registerWidgetToMode, this, _1, _2, _3, _4, _5, _6);
    }
    if (!windowService->registerWidget) {
        windowService->registerWidget = std::bind(&Controller::registerWidget, this, _1, _2);
    }
    if (!windowService->registerWidgetCreator) {
        windowService->registerWidgetCreator = std::bind(&Controller::registerWidgetCreator, this, _1, _2);
    }
    if (!windowService->showWidgetAtPosition) {
        windowService->showWidgetAtPosition = std::bind(&Controller::showWidgetAtPosition, this, _1, _2, _3);
    }
    if (!windowService->setDockHeaderName) {
        windowService->setDockHeaderName = std::bind(&Controller::setDockHeaderName, this, _1, _2);
    }
    if (!windowService->setDockHeaderList) {
        windowService->setDockHeaderList = std::bind(&Controller::setDockHeaderList, this, _1, _2);
    }
    if (!windowService->deleteDockHeader) {
        windowService->deleteDockHeader = std::bind(&Controller::deleteDockHeader, this, _1);
    }
    if (!windowService->addToolBtnToDockHeader) {
        windowService->addToolBtnToDockHeader = std::bind(&MainWindow::addWidgetToDockHeader, d->mainWindow, _1, _2);
    }
    if (!windowService->setDockWidgetFeatures) {
        windowService->setDockWidgetFeatures = std::bind(&MainWindow::setDockWidgetFeatures, d->mainWindow, _1, _2);
    }
    if (!windowService->splitWidgetOrientation) {
        windowService->splitWidgetOrientation = std::bind(&MainWindow::splitWidgetOrientation, d->mainWindow, _1, _2, _3);
    }
    if (!windowService->addNavigationItem) {
        windowService->addNavigationItem = std::bind(&Controller::addNavigationItem, this, _1, _2);
    }
    if (!windowService->addNavigationItemToBottom) {
        windowService->addNavigationItemToBottom = std::bind(&Controller::addNavigationItemToBottom, this, _1, _2);
    }
    if (!windowService->switchWidgetNavigation) {
        windowService->switchWidgetNavigation = std::bind(&Controller::switchWidgetNavigation, this, _1);
    }
    if (!windowService->bindWidgetToNavigation) {
        windowService->bindWidgetToNavigation = std::bind(&Controller::bindWidgetToNavigation, this, _1, _2);
    }
    if (!windowService->getAllNavigationItemName) {
        windowService->getAllNavigationItemName = std::bind(&NavigationBar::getAllNavigationItemName, d->navigationBar);
    }
    if (!windowService->getPriorityOfNavigationItem) {
        windowService->getPriorityOfNavigationItem = std::bind(&NavigationBar::getPriorityOfNavigationItem, d->navigationBar, _1);
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
    if (!windowService->addWidgetToTopTool) {
        windowService->addWidgetToTopTool = std::bind(&Controller::addWidgetToTopTool, this, _1, _2, _3, _4);
    }
    if (!windowService->addTopToolItem) {
        windowService->addTopToolItem = std::bind(&Controller::addTopToolItem, this, _1, _2, _3);
    }
    if (!windowService->addTopToolItemToRight) {
        windowService->addTopToolItemToRight = std::bind(&Controller::addTopToolItemToRight, this, _1, _2, _3);
    }
    if (!windowService->showTopToolBar) {
        windowService->showTopToolBar = std::bind(&Controller::showTopToolBar, this);
    }
    if (!windowService->setTopToolItemVisible) {
        windowService->setTopToolItemVisible = std::bind(&Controller::setTopToolItemVisible, this, _1, _2);
    }
    if (!windowService->removeTopToolItem) {
        windowService->removeTopToolItem = std::bind(&Controller::removeTopToolItem, this, _1);
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
    if (!windowService->addStatusBarItem) {
        windowService->addStatusBarItem = std::bind(&Controller::addStatusBarItem, this, _1);
    }
    if (!windowService->addWidgetWorkspace) {
        windowService->addWidgetWorkspace = std::bind(&WorkspaceWidget::addWorkspaceWidget, d->workspace, _1, _2, _3);
    }
    if (!windowService->addWidgetRightspace) {
        windowService->addWidgetRightspace = std::bind(&WorkspaceWidget::addWorkspaceWidget, d->rightspace, _1, _2, _3);
    }
    if (!windowService->registerToolBtnToWorkspaceWidget) {
        windowService->registerToolBtnToWorkspaceWidget = std::bind(&WorkspaceWidget::registerToolBtnToWidget, d->workspace, _1, _2);
    }
    if (!windowService->registerToolBtnToRightspaceWidget) {
        windowService->registerToolBtnToRightspaceWidget = std::bind(&WorkspaceWidget::registerToolBtnToWidget, d->rightspace, _1, _2);
    }
    if (!windowService->showWidgetAtRightspace) {
        windowService->showWidgetAtRightspace = std::bind(&WorkspaceWidget::switchWidgetWorkspace, d->rightspace, _1);
    }
    if (!windowService->registerWidgetToDockHeader) {
        windowService->registerWidgetToDockHeader = std::bind(&Controller::registerWidgetToDockHeader, this, _1, _2);
    }
    if (!windowService->createFindPlaceHolder) {
        windowService->createFindPlaceHolder = std::bind(&PlaceHolderManager::createPlaceHolder, PlaceHolderManager::instance(), _1, _2);
    }
    if (!windowService->getCentralWidgetName) {
        windowService->getCentralWidgetName = std::bind(&MainWindow::getCentralWidgetName, d->mainWindow);
    }
    if (!windowService->getCurrentDockName) {
        windowService->getCurrentDockName = std::bind(&MainWindow::getCurrentDockName, d->mainWindow, _1);
    }
    if (!windowService->resizeDocks) {
        windowService->resizeDocks = std::bind(&Controller::resizeDocks, this, _1, _2, _3);
    }
}

Controller::~Controller()
{
    if (d->mainWindow)
        delete d->mainWindow;
    foreach (auto module, d->modules.values()) {
        delete module;
        module = nullptr;
    }
    if (d)
        delete d;
}

void Controller::createDockWidget(WidgetInfo &info)
{
    auto dock = d->mainWindow->addWidget(info.name, info.getDWidget(), info.defaultPos);
    info.dockWidget = dock;
    info.created = true;

    if (!info.headerName.isEmpty())
        d->mainWindow->setDockHeaderName(info.name, info.headerName);
    else if (!info.headerList.isEmpty())
        d->mainWindow->setDockHeaderList(info.name, info.headerList);

    if (!info.showHeader) {
        d->mainWindow->deleteDockHeader(info.name);
    } else if (!info.headerWidget.isEmpty()) {
        for (auto btn : info.headerWidget)
            d->mainWindow->addWidgetToDockHeader(info.name, btn);
    }

    if (info.icon.isNull())
        info.icon = QIcon::fromTheme("default_dock");
}

void Controller::raiseMode(const QString &mode)
{
    if (!d->validModeList.contains(mode)) {
        qWarning() << "mode can only choose CM_RECENT / CM_EDIT / CM_DEBUG";
        return;
    }

    auto widgetList = d->modeInfo[mode];
    foreach (auto widgetName, widgetList) {
        auto &widgetInfo = d->allWidgets[widgetName];
        if (widgetInfo.replace)
            d->mainWindow->hideWidget(widgetInfo.defaultPos);
        if (!widgetInfo.hiddenByManual)
            d->mainWindow->showWidget(widgetInfo.name);
        // widget in mainWindow is Dock(widget), show dock and hide widget
        widgetInfo.getDWidget()->setVisible(widgetInfo.defaultVisible);
        if (widgetInfo.dockWidget)
            d->currentDocks.append(widgetInfo.name);
    }

    checkDocksManager();

    if (mode == CM_RECENT) {
        d->mode = mode;
        uiController.modeRaised(CM_RECENT);
        return;
    }

    showStatusBar();

    if (mode == CM_EDIT)
        showWorkspace();

    showTopToolBar();
    showContextWidget();
    showRightspace();

    d->mode = mode;
    uiController.modeRaised(mode);
}

void Controller::replaceWidget(const QString &name, Position pos)
{
    showWidgetAtPosition(name, pos, true);
}

void Controller::insertWidget(const QString &name, Position pos, Qt::Orientation orientation)
{
    if (!d->allWidgets.contains(name)) {
        qWarning() << "no widget named:" << name;
        return;
    }

    auto &info = d->allWidgets[name];
    if (!info.created) {
        auto dock = d->mainWindow->addWidget(name, info.getDWidget(), pos, orientation);
        info.dockWidget = dock;
        info.created = true;
        info.replace = false;
    } else {
        d->mainWindow->showWidget(name);
    }
}

void Controller::hideWidget(const QString &name)
{
    d->mainWindow->hideWidget(name);
}

void Controller::registerWidgetToMode(const QString &name, AbstractWidget *abstractWidget, const QString &mode, Position pos, bool replace, bool isVisible)
{
    if (!d->validModeList.contains(mode)) {
        qWarning() << "mode can only choose CM_RECENT / CM_EDIT / CM_DEBUG";
        return;
    }

    if (d->allWidgets.contains(name)) {
        auto &info = d->allWidgets[name];
        if (!d->modeInfo[mode].contains(name)) {
            if (info.defaultPos != pos)
                qWarning() << "widget named: " << name << "has registed to another position";
            d->modeInfo[mode].append(name);
        } else {
            qWarning() << "Widget named: " << name << "has alreay registed";
        }
        return;
    }

    DWidget *qWidget = static_cast<DWidget *>(abstractWidget->qWidget());
    if (!qWidget->parent())
        qWidget->setParent(d->mainWindow);

    WidgetInfo widgetInfo;
    widgetInfo.name = name;
    widgetInfo.defaultPos = pos;
    widgetInfo.replace = replace;
    widgetInfo.setWidget(qWidget);
    widgetInfo.defaultVisible = isVisible;
    widgetInfo.icon = abstractWidget->getDisplayIcon();

    createDockWidget(widgetInfo);
    d->mainWindow->hideWidget(name);

    d->allWidgets.insert(name, widgetInfo);
    d->modeInfo[mode].append(name);
}

void Controller::registerWidget(const QString &name, AbstractWidget *abstractWidget)
{
    if (d->allWidgets.contains(name))
        return;

    auto widget = static_cast<DWidget *>(abstractWidget->qWidget());
    if (!widget->parent())
        widget->setParent(d->mainWindow);

    WidgetInfo widgetInfo;
    widgetInfo.name = name;
    widgetInfo.setWidget(widget);
    widgetInfo.icon = abstractWidget->getDisplayIcon();

    d->allWidgets.insert(name, widgetInfo);
}

void Controller::registerWidgetCreator(const QString &name, std::function<AbstractWidget*()> &widgetCreateFunc)
{
     if (d->allWidgets.contains(name))
        return;

    WidgetInfo widgetInfo;
    widgetInfo.name = name;
    widgetInfo.createDWidgetFunc = widgetCreateFunc;

    d->allWidgets.insert(name, widgetInfo);
}

void Controller::showWidgetAtPosition(const QString &name, Position pos, bool replace)
{
    if (replace)
        d->mainWindow->hideWidget(pos);

    if (!d->allWidgets.contains(name)) {
        qWarning() << "no widget named: " << name;
        return;
    }

    auto &info = d->allWidgets[name];
    if (!info.created) {
        info.defaultPos = pos;
        info.replace = replace;
        createDockWidget(info);
    } else if (!info.hiddenByManual || d->widgetBindToNavigation.contains(name)) {
        d->mainWindow->showWidget(name);
        info.hiddenByManual = false;
    }

    if (pos != Position::Central && pos != Position::FullWindow) {
        if (replace) {
            for (auto name : d->currentDocks) {
                if (d->mainWindow->positionOfDock(name) == pos)
                    d->currentDocks.removeOne(name);
            }
        }
        d->currentDocks.append(name);
    }

    checkDocksManager();
}

void Controller::resizeDocks(const QList<QString> &docks, const QList<int> &sizes, Qt::Orientation orientation)
{
    QList<QDockWidget *> dockWidgets;
    for (auto dockName : docks) {
        if (d->allWidgets.contains(dockName) && d->allWidgets[dockName].created)
            dockWidgets.append(d->allWidgets[dockName].dockWidget);
        else
            qWarning() << "Dock named: " << dockName << "has not created!";
    }

    d->mainWindow->resizeDocks(dockWidgets, sizes, orientation);
    QApplication::processEvents();   // process layout update
}

void Controller::setDockHeaderName(const QString &dockName, const QString &headerName)
{
    if (!d->allWidgets.contains(dockName)) {
        qWarning() << "No widget named: " << dockName;
        return;
    }

    auto &info = d->allWidgets[dockName];
    info.headerName = headerName;

    if (info.created)
        d->mainWindow->setDockHeaderName(dockName, headerName);
}

void Controller::deleteDockHeader(const QString &dockName)
{
    if (!d->allWidgets.contains(dockName)) {
        qWarning() << "No widget named: " << dockName;
        return;
    }
    auto &info = d->allWidgets[dockName];
    if (info.created)
        d->mainWindow->deleteDockHeader(dockName);
    else
        info.showHeader = false;
}

void Controller::setDockHeaderList(const QString &dockName, const QList<QAction *> &actions)
{
    if (!d->allWidgets.contains(dockName)) {
        qWarning() << "No widget named: " << dockName;
        return;
    }

    auto &info = d->allWidgets[dockName];
    info.headerName = "";
    info.headerList = actions;

    if (info.created)
        d->mainWindow->setDockHeaderList(dockName, actions);
}

void Controller::addNavigationItem(AbstractAction *action, quint8 priority)
{
    if (!action)
        return;
    auto inputAction = action->qAction();

    d->navigationBar->addNavItem(inputAction, NavigationBar::top, priority);
    d->navigationActions.insert(inputAction->text(), inputAction);
}

void Controller::addNavigationItemToBottom(AbstractAction *action, quint8 priority)
{
    if (!action)
        return;
    auto inputAction = action->qAction();

    d->navigationBar->addNavItem(inputAction, NavigationBar::bottom, priority);
    d->navigationActions.insert(inputAction->text(), inputAction);
}

void Controller::switchWidgetNavigation(const QString &navName)
{
    d->navigationBar->setNavActionChecked(navName, true);
    if (d->currentNavigation == navName) {
        auto action = d->navigationActions[navName];
        if (d->widgetBindToNavigation.values().contains(action)) {
            auto dockName = d->widgetBindToNavigation.key(action);
            auto &info = d->allWidgets[dockName];
            if (info.hiddenByManual)
                d->mainWindow->showWidget(dockName);
            info.hiddenByManual = false;
        }
        return;
    }
    d->currentNavigation = navName;

    d->mainWindow->hideAllWidget();
    d->mainWindow->hideTopTollBar();
    hideStatusBar();

    d->currentDocks.clear();
    for (auto btn : d->dockButtons.values())
        btn->hide();

    if (d->modePluginMap.values().contains(navName))
        raiseMode(d->modePluginMap.key(navName));
    d->navigationActions[navName]->trigger();

    // send event
    uiController.switchToWidget(navName);
}

void Controller::bindWidgetToNavigation(const QString &dockName, AbstractAction *abstractAction)
{
    auto action = abstractAction->qAction();
    if (!action || d->widgetBindToNavigation.values().contains(action)) {
        qWarning() << action->text() << " Navigation has already bind to a widget or action invalid";
        return;
    }

    if (!d->allWidgets.contains(dockName)) {
        qWarning() << "no widget named: " << dockName;
        return;
    }

    d->widgetBindToNavigation.insert(dockName, action);
}

void Controller::registerWidgetToDockHeader(const QString &dockName, QWidget *widget)
{
    if (!d->allWidgets.contains(dockName)) {
        qWarning() << "No widget named: " << dockName;
        return;
    }

    auto &info = d->allWidgets[dockName];
    if (info.created)
        d->mainWindow->addWidgetToDockHeader(dockName, widget);
    info.headerWidget.append(widget);
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
    tabBtn->setFixedHeight(28);
    tabBtn->setFlat(true);
    tabBtn->setFocusPolicy(Qt::NoFocus);
    DFontSizeManager *fontSizeManager = DFontSizeManager::instance();
    QFont font = fontSizeManager->t7();
    tabBtn->setFont(font);
    if (!isVisible)
        tabBtn->hide();

    d->contextButtonLayout->addWidget(tabBtn);
    connect(tabBtn, &DPushButton::clicked, qWidget, [=] {
        switchContextWidget(title);
    });

    d->tabButtons.insert(title, tabBtn);
}

void Controller::showContextWidget()
{
    auto contextInfo = d->allWidgets[WN_CONTEXTWIDGET];
    if (!contextInfo.created) {
        createDockWidget(d->allWidgets[WN_CONTEXTWIDGET]);
        d->mainWindow->deleteDockHeader(WN_CONTEXTWIDGET);
    } else if (!contextInfo.hiddenByManual) {
        d->mainWindow->showWidget(WN_CONTEXTWIDGET);
    }
    d->currentDocks.append(WN_CONTEXTWIDGET);
}

void Controller::showRightspace()
{
    auto &rightSpaceInfo = d->allWidgets[WN_RIGHTSPACE];
    if (!rightSpaceInfo.created) {
        createDockWidget(d->allWidgets[WN_RIGHTSPACE]);
        d->mainWindow->showWidget(WN_RIGHTSPACE);
        d->mainWindow->resizeDock(WN_RIGHTSPACE, QSize(300, 300));

        for (auto btn : d->rightspace->getAllToolBtn())
            d->mainWindow->addWidgetToDockHeader(WN_RIGHTSPACE, btn);

        auto titles = d->rightspace->allWidgetTitles();
        QList<QAction *> headers;
        for (auto title : titles) {
            QAction *action = new QAction(title, d->rightspace);
            connect(action, &QAction::triggered, this, [=]() { d->rightspace->switchWidgetWorkspace(title); });
            headers.append(action);
        }
        d->mainWindow->setDockHeaderList(WN_RIGHTSPACE, headers);
        d->mainWindow->setDockHeaderName(WN_RIGHTSPACE, d->rightspace->currentTitle());

        d->rightspace->addedToController = true;
        connect(d->rightspace, &WorkspaceWidget::workSpaceWidgeSwitched, this, [=](const QString &title) {
            d->mainWindow->setDockHeaderName(WN_RIGHTSPACE, title);
        });
        connect(rightSpaceInfo.dockWidget, &QDockWidget::visibilityChanged, d->showRightspaceBtn, [=](bool visible){ d->showRightspaceBtn->setChecked(visible); });
    } else if (!rightSpaceInfo.hiddenByManual) {
        d->mainWindow->showWidget(WN_RIGHTSPACE);
    }
    d->currentDocks.append(WN_RIGHTSPACE);
}

bool Controller::hasContextWidget(const QString &title)
{
    return d->contextWidgets.contains(title);
}

void Controller::hideContextWidget()
{
    d->mainWindow->hideWidget(WN_CONTEXTWIDGET);
}

void Controller::switchContextWidget(const QString &title)
{
    if (!d->contextWidgets.contains(title)) {
        qWarning() << "No ContextWidget named: " << title;
        return;
    }

    d->stackContextWidget->setCurrentWidget(d->contextWidgets[title]);
    if (d->stackContextWidget->isHidden()) {
        d->contextWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        d->stackContextWidget->show();
    }
    if (d->tabButtons.contains(title))
        d->tabButtons[title]->show();

    for (auto it = d->tabButtons.begin(); it != d->tabButtons.end(); ++it) {
        it.value()->setChecked(false);
        if (it.key() == title)
            it.value()->setChecked(true);
    }
}

void Controller::addWidgetToTopTool(AbstractWidget *abstractWidget, bool addSeparator, bool addToLeft, quint8 priority)
{
    if (!abstractWidget)
        return;
    auto widget = static_cast<DWidget *>(abstractWidget->qWidget());
    if (!widget)
        return;

    QHBoxLayout *hlayout { nullptr };

    if (!addToLeft) {
        hlayout = qobject_cast<QHBoxLayout *>(d->rightTopToolBar->layout());
    } else {
        hlayout = qobject_cast<QHBoxLayout *>(d->leftTopToolBar->layout());
    }

    // sort
    auto index = 0;
    widget->setProperty("toptool_priority", priority);
    for (; index < hlayout->count(); index++) {
        if (hlayout->itemAt(index)->isEmpty())
            continue;
        auto w = hlayout->itemAt(index)->widget();
        if (priority <= w->property("toptool_priority").toInt())
            break;
    }

    if (addSeparator) {
        DWidget *separator = new DWidget(d->mainWindow);
        DVerticalLine *line = new DVerticalLine(d->mainWindow);
        auto separatorLayout = new QHBoxLayout(separator);
        separator->setProperty("toptool_priority", priority - 1);
        line->setFixedHeight(20);
        line->setFixedWidth(1);

        separatorLayout->setContentsMargins(5, 0, 5, 0);
        separatorLayout->addWidget(line);

        hlayout->insertWidget(index++, separator);
    }

    hlayout->insertWidget(index, widget);
}

DToolButton *Controller::addTopToolItem(Command *action, bool addSeparator, quint8 priority)
{
    if (!action || !action->action())
        return nullptr;

    auto iconBtn = createIconButton(action->action());
    addWidgetToTopTool(new AbstractWidget(iconBtn), addSeparator, true, priority);
    return iconBtn;
}

DToolButton *Controller::addTopToolItemToRight(Command *action, bool addSeparator, quint8 priority)
{
    if (!action || !action->action())
        return nullptr;

    auto iconBtn = createIconButton(action->action());
    addWidgetToTopTool(new AbstractWidget(iconBtn), addSeparator, false, priority);
    return iconBtn;
}

void Controller::showTopToolBar()
{
    d->mainWindow->showTopToolBar();
}

void Controller::openFileDialog()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = DFileDialog::getOpenFileName(nullptr, tr("Open Document"), dir);
    if (filePath.isEmpty() && !QFileInfo(filePath).exists())
        return;
    recent.saveOpenedFile(filePath);
    editor.openFile(QString(), filePath);
}

void Controller::loading()
{
    d->loadingwidget = new loadingWidget(d->mainWindow);
    d->mainWindow->addWidget(WN_LOADINGWIDGET, d->loadingwidget);

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                     this, [=]() {
                         d->mainWindow->removeWidget(WN_LOADINGWIDGET);

                         d->navigationToolBar->show();
                         d->mainWindow->setToolbar(Qt::ToolBarArea::LeftToolBarArea, d->navigationToolBar);
                     });
}

void Controller::initMainWindow()
{
    qInfo() << __FUNCTION__;
    if (!d->mainWindow) {
        d->mainWindow = new MainWindow;
        d->mainWindow->setObjectName("MainWindow");
        d->mainWindow->setMinimumSize(MW_MIN_WIDTH, MW_MIN_HEIGHT);
        new ActionManager(this);
        registerDefaultContainers();
        registerDefaultActions();

        QString initFile = CustomPaths::user(CustomPaths::Configures) + "/mainwindow.ini";
        QFile file(initFile);
        if (file.open(QFile::ReadOnly)) {
            QByteArray ba;
            QDataStream inFile(&file);
            inFile >> ba;
            d->mainWindow->restoreGeometry(ba);
            d->mainWindow->show();
        } else {
            d->mainWindow->resize(MW_WIDTH, MW_HEIGHT);
            d->mainWindow->showMaximized();
        }

        if (CommandParser::instance().getModel() == CommandParser::CommandLine) {
            d->mainWindow->hide();
        }

        loading();

        auto desktop = QApplication::desktop();
        int currentScreenIndex = desktop->screenNumber(d->mainWindow);
        QList<QScreen *> screenList = QGuiApplication::screens();

        if (currentScreenIndex < screenList.count()) {
            QRect screenRect = screenList[currentScreenIndex]->geometry();
            int screenWidth = screenRect.width();
            int screenHeight = screenRect.height();
            d->mainWindow->move((screenWidth - d->mainWindow->width()) / 2, (screenHeight - d->mainWindow->height()) / 2);
        }

        connect(d->mainWindow, &MainWindow::dockHidden, this, [=](const QString &dockName) {
            if (d->allWidgets.contains(dockName)) {
                auto &info = d->allWidgets[dockName];
                info.hiddenByManual = true;
                if (d->widgetBindToNavigation.contains(dockName))
                    d->navigationBar->setNavActionChecked(d->widgetBindToNavigation[dockName]->text(), false);
            }
        });
    }
}

void Controller::initNavigationBar()
{
    qInfo() << __FUNCTION__;
    if (d->navigationBar)
        return;
    d->navigationToolBar = new DWidget(d->mainWindow);
    auto vLayout = new QVBoxLayout(d->navigationToolBar);
    d->navigationBar = new NavigationBar(d->mainWindow);
    d->navigationToolBar->hide();

    vLayout->addWidget(d->navigationBar);
    vLayout->setContentsMargins(0, 0, 1, 0);
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
    d->contextTabBar->setFixedHeight(40);

    d->contextButtonLayout = new QHBoxLayout;
    d->contextButtonLayout->setSpacing(0);
    d->contextButtonLayout->setContentsMargins(12, 6, 12, 6);
    d->contextButtonLayout->setAlignment(Qt::AlignLeft);

    DToolButton *hideBtn = new DToolButton(d->contextTabBar);
    hideBtn->setFixedSize(35, 35);
    hideBtn->setIcon(QIcon::fromTheme("hide_dock"));
    hideBtn->setToolTip(tr("Hide ContextWidget"));
    connect(hideBtn, &DToolButton::clicked, d->contextWidget, [=]() {
        if (d->stackContextWidget->isVisible()) {
            d->stackContextWidget->hide();
            d->contextWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            d->mainWindow->resizeDock(WN_CONTEXTWIDGET, d->contextTabBar->size());
        } else {
            d->contextWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            d->stackContextWidget->show();
        }
    });

    QHBoxLayout *tabbarLayout = new QHBoxLayout(d->contextTabBar);
    tabbarLayout->setContentsMargins(0, 0, 0, 0);
    tabbarLayout->addLayout(d->contextButtonLayout);
    tabbarLayout->addWidget(hideBtn, Qt::AlignRight);

    QVBoxLayout *contextVLayout = new QVBoxLayout;
    contextVLayout->setContentsMargins(0, 0, 0, 0);
    contextVLayout->setSpacing(0);
    contextVLayout->addWidget(d->contextTabBar);
    contextVLayout->addWidget(new DHorizontalLine);
    contextVLayout->addWidget(d->stackContextWidget);
    d->contextWidget->setLayout(contextVLayout);

    // add contextWidget after add centralWidget or it`s height is incorrect
    WidgetInfo info;
    info.name = WN_CONTEXTWIDGET;
    info.setWidget(d->contextWidget);
    info.defaultPos = Position::Bottom;
    info.icon = QIcon::fromTheme("context_widget");

    if (d->statusBar) {
        d->showContextBtn = createDockButton(info);
        d->showContextBtn->setChecked(true);
        d->statusBar->insertPermanentWidget(0, d->showContextBtn);
    }

    d->allWidgets.insert(WN_CONTEXTWIDGET, info);
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

    WidgetInfo info;
    info.name = WN_WORKSPACE;
    info.setWidget(d->workspace);
    info.defaultPos = Position::Left;
    info.replace = true;
    d->allWidgets.insert(WN_WORKSPACE, info);
}

void Controller::initRightspaceWidget()
{
    if (d->rightspace)
        return;

    d->rightspace = new WorkspaceWidget(d->mainWindow);

    WidgetInfo info;
    info.name = WN_RIGHTSPACE;
    info.setWidget(d->rightspace);
    info.defaultPos = Position::Right;
    info.replace = true;
    info.icon = QIcon::fromTheme("uc_right_show");
    d->allWidgets.insert(WN_RIGHTSPACE, info);

    d->showRightspaceBtn = createDockButton(info);
    d->showRightspaceBtn->setChecked(true);
    auto scAction = new QAction(tr("Open rightspace"), d->rightspace);
    auto cmd = ActionManager::instance()->registerAction(scAction, "Core.Open.Rightspace");
    cmd->setDefaultKeySequence(QKeySequence(Qt::ALT + Qt::Key_L));
    connect(scAction, &QAction::triggered, d->showRightspaceBtn, &DToolButton::clicked);

    d->statusBar->insertPermanentWidget(0, d->showRightspaceBtn);
}

void Controller::initTopToolBar()
{
    d->leftTopToolBar = new DWidget(d->mainWindow);
    auto hlayout = new QHBoxLayout(d->leftTopToolBar);
    hlayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0, 0, 0, 0);

    d->locatorBar = LocatorManager::instance()->getInputEdit();
    d->rightTopToolBar = new DWidget(d->mainWindow);

    QHBoxLayout *rtLayout = new QHBoxLayout(d->rightTopToolBar);
    rtLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rtLayout->setContentsMargins(0, 0, 0, 0);
    rtLayout->setSpacing(10);

    d->mainWindow->setLeftTopToolWidget(d->leftTopToolBar);
    d->mainWindow->setMiddleTopToolWidget(d->locatorBar);
    d->mainWindow->setRightTopToolWidget(d->rightTopToolBar);

    d->mainWindow->hideTopTollBar();
}

void Controller::initModules()
{
    for (auto module : d->modules.values()) {
        module->initialize(this);
    }
}

void Controller::initDocksManager()
{
    d->docksManager = new DocksManagerButton(d->navigationToolBar, this);
    d->docksManager->setIcon(QIcon::fromTheme("docks_manager"));
    d->docksManager->setFocusPolicy(Qt::NoFocus);
    d->docksManager->setToolTip(tr("Show docks in this view"));
    d->docksManager->hide();
    d->navigationBar->addNavButton(d->docksManager, NavigationBar::bottom, Priority::low);

    connect(d->navigationBar, &NavigationBar::leave, this, [=]() {
        for (auto btn : d->dockButtons.values())
            btn->hide();
    });
}

void Controller::registerDefaultContainers()
{
    ActionManager::instance()->setContext({ C_GLOBAL });
    auto titleContainer = ActionManager::instance()->createContainer(M_TITLEBAR);
    titleContainer->appendGroup(G_FILE);
    titleContainer->appendGroup(G_EDIT);
    titleContainer->appendGroup(G_BUILD);
    titleContainer->appendGroup(G_DEBUG);
    titleContainer->appendGroup(G_TOOLS);
    titleContainer->appendGroup(G_HELP);

    titleContainer->addSeparator(G_BUILD);
    titleContainer->addSeparator(G_TOOLS);
    d->mainWindow->titlebar()->setMenu(titleContainer->menu());

    // file menu
    auto fileContainer = ActionManager::instance()->createContainer(M_FILE);
    fileContainer->menu()->setTitle(tr("&File"));
    fileContainer->appendGroup(G_FILE_NEW);
    fileContainer->appendGroup(G_FILE_OPEN);
    fileContainer->appendGroup(G_FILE_CLOSE);
    fileContainer->appendGroup(G_FILE_SAVE);

    fileContainer->addSeparator(G_FILE_NEW);
    fileContainer->addSeparator(G_FILE_OPEN);
    fileContainer->addSeparator(G_FILE_CLOSE);
    fileContainer->addSeparator(G_FILE_SAVE);
    titleContainer->addMenu(fileContainer, G_FILE);

    // edit menu
    auto editContainer = ActionManager::instance()->createContainer(M_EDIT);
    editContainer->menu()->setTitle(tr("&Edit"));
    editContainer->appendGroup(G_EDIT_UNDOREDO);
    editContainer->appendGroup(G_EDIT_COPYPASTE);
    editContainer->appendGroup(G_EDIT_SELECTALL);
    editContainer->appendGroup(G_EDIT_FIND);
    editContainer->appendGroup(G_EDIT_OTHER);

    editContainer->addSeparator(G_EDIT_COPYPASTE);
    editContainer->addSeparator(G_EDIT_SELECTALL);
    editContainer->addSeparator(G_EDIT_FIND);
    titleContainer->addMenu(editContainer, G_EDIT);

    // build menu
    auto buildContainer = ActionManager::instance()->createContainer(M_BUILD);
    buildContainer->menu()->setTitle(tr("&Build"));
    titleContainer->addMenu(buildContainer, G_BUILD);

    // debug menu
    auto debugContainer = ActionManager::instance()->createContainer(M_DEBUG);
    debugContainer->menu()->setTitle(tr("&Debug"));
    titleContainer->addMenu(debugContainer, G_DEBUG);

    // tools menu
    auto toolsContainer = ActionManager::instance()->createContainer(M_TOOLS);
    toolsContainer->menu()->setTitle(tr("&Tools"));
    titleContainer->addMenu(toolsContainer, G_TOOLS);

    // help menu
    auto helpContainer = ActionManager::instance()->createContainer(M_HELP);
    helpContainer->menu()->setTitle(tr("&Help"));
    titleContainer->addMenu(helpContainer, G_HELP);
}

void Controller::registerDefaultActions()
{
    auto mFile = ActionManager::instance()->actionContainer(M_FILE);
    auto mHelp = ActionManager::instance()->actionContainer(M_HELP);

    // file actions
    QAction *act = new QAction(tr("Open File"), mFile);
    auto cmd = ActionManager::instance()->registerAction(act, A_OPEN_FILE);
    cmd->setDefaultKeySequence(Qt::CTRL | Qt::Key_O);
    mFile->addAction(cmd, G_FILE_OPEN);
    connect(act, &QAction::triggered, this, &Controller::openFileDialog);

    act = new QAction(tr("Open Project"), mFile);
    cmd = ActionManager::instance()->registerAction(act, A_OPEN_PROJECT);
    mFile->addAction(cmd, G_FILE_OPEN);
    connect(act, &QAction::triggered, this, [=]() {
        auto prjService = dpfGetService(ProjectService);
        prjService->openProject();
    });

    // help actions
    act = new QAction(tr("Report Bug"), mHelp);
    cmd = ActionManager::instance()->registerAction(act, A_REPORTBUG);
    mHelp->addAction(cmd);
    connect(act, &QAction::triggered, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://github.com/linuxdeepin/deepin-unioncode/issues"));
    });

    act = new QAction(tr("Help Documents"), mHelp);
    cmd = ActionManager::instance()->registerAction(act, A_HELPDOC);
    mHelp->addAction(cmd);
    connect(act, &QAction::triggered, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://uosdn.uniontech.com/#document2?dirid=656d40a9bd766615b0b02e5e"));
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

void Controller::addStatusBarItem(QWidget *item)
{
    if (d->statusBar && item) {
        if (!item->parent())
            item->setParent(d->statusBar);
        d->statusBar->insertPermanentWidget(0, item);
    }
}

void Controller::switchWorkspace(const QString &titleName)
{
    if (d->workspace)
        d->workspace->switchWidgetWorkspace(titleName);
}

void Controller::showWorkspace()
{
    auto &workSpaceInfo = d->allWidgets[WN_WORKSPACE];
    if (!workSpaceInfo.created) {
        createDockWidget(d->allWidgets[WN_WORKSPACE]);
        d->mainWindow->showWidget(WN_WORKSPACE);
        d->mainWindow->resizeDock(WN_WORKSPACE, QSize(300, 300));

        for (auto btn : d->workspace->getAllToolBtn())
            d->mainWindow->addWidgetToDockHeader(WN_WORKSPACE, btn);

        DToolButton *expandAll = new DToolButton(d->workspace);
        expandAll->setToolTip(tr("Expand All"));
        expandAll->setIcon(QIcon::fromTheme("expand_all"));
        d->mainWindow->addWidgetToDockHeader(WN_WORKSPACE, expandAll);
        connect(expandAll, &DToolButton::clicked, this, []() { workspace.expandAll(); });

        DToolButton *foldAll = new DToolButton(d->workspace);
        foldAll->setToolTip(tr("Fold All"));
        foldAll->setIcon(QIcon::fromTheme("collapse_all"));
        d->mainWindow->addWidgetToDockHeader(WN_WORKSPACE, foldAll);
        connect(foldAll, &DToolButton::clicked, this, []() { workspace.foldAll(); });

        expandAll->setVisible(d->workspace->getCurrentExpandState());
        foldAll->setVisible(d->workspace->getCurrentExpandState());

        auto titles = d->workspace->allWidgetTitles();
        QList<QAction *> headers;
        for (auto title : titles) {
            QAction *action = new QAction(title, d->workspace);
            connect(action, &QAction::triggered, this, [=]() { d->workspace->switchWidgetWorkspace(title); });
            headers.append(action);
        }
        d->mainWindow->setDockHeaderList(WN_WORKSPACE, headers);
        d->mainWindow->setDockHeaderName(WN_WORKSPACE, d->workspace->currentTitle());

        connect(d->workspace, &WorkspaceWidget::expandStateChange, this, [=](bool canExpand) {
            expandAll->setVisible(canExpand);
            foldAll->setVisible(canExpand);
        });

        d->workspace->addedToController = true;
        bindWidgetToNavigation(WN_WORKSPACE, new AbstractAction(d->navigationActions[MWNA_EDIT]));

        connect(d->workspace, &WorkspaceWidget::workSpaceWidgeSwitched, this, [=](const QString &title) {
            d->mainWindow->setDockHeaderName(WN_WORKSPACE, title);
        });
    } else {
        d->mainWindow->showWidget(WN_WORKSPACE);
    }
    d->currentDocks.append(WN_WORKSPACE);
}

DToolButton *Controller::createIconButton(QAction *action)
{
    auto iconBtn = utils::createIconButton(action, d->mainWindow);
    d->topToolBtn.insert(action, iconBtn);
    return iconBtn;
}

void Controller::removeTopToolItem(Command *action)
{
    if (!action || !action->action())
        return;

    auto iconBtn = d->topToolBtn.value(action->action());

    delete iconBtn;
    d->topToolBtn.remove(action->action());
}

bool Controller::checkDocksManager()
{
    bool ret = false;
    for (auto dock : d->currentDocks) {
        auto &dockInfo = d->allWidgets[dock];
        if (!dockInfo.dockWidget || d->widgetBindToNavigation.contains(dock) || dockInfo.name == WN_CONTEXTWIDGET)
            continue;

        if (!d->dockButtons.contains(dock)) {
            auto btn = createDockButton(dockInfo);
            btn->hide();
            d->navigationBar->addNavButton(btn, NavigationBar::bottom, Priority::high);
            d->dockButtons.insert(dockInfo.name, btn);
        }

        d->docksManager->show();
        ret = true;
    }

    if (!ret)
        d->docksManager->hide();
    return ret;
}

void Controller::showCurrentDocksManager()
{
    for (auto dock : d->currentDocks) {
        auto &dockInfo = d->allWidgets[dock];
        if (!d->dockButtons.contains(dockInfo.name))
            continue;

        auto btn = d->dockButtons[dockInfo.name];
        btn->show();
        btn->setChecked(dockInfo.dockWidget->isVisible());
    }
}

DToolButton *Controller::createDockButton(const WidgetInfo &info)
{
    DToolButton *btn = new DToolButton(d->navigationToolBar);
    btn->setIcon(info.icon);
    btn->setToolTip(info.headerName.isEmpty() ? info.name : info.headerName);
    btn->setCheckable(true);
    connect(btn, &DToolButton::clicked, this, [=]() {
        auto &dockInfo = d->allWidgets[info.name];
        if (!dockInfo.dockWidget)
            return;
        if (dockInfo.dockWidget->isVisible()) {
            d->mainWindow->hideWidget(dockInfo.name);
            btn->setChecked(false);
            dockInfo.hiddenByManual = true;
        } else {
            d->mainWindow->showWidget(dockInfo.name);
            btn->setChecked(true);
            dockInfo.hiddenByManual = false;
        }
    },
            Qt::UniqueConnection);
    return btn;
}

void Controller::setTopToolItemVisible(Command *action, bool visible)
{
    if (!action || !action->action())
        return;

    auto iconBtn = d->topToolBtn.value(action->action());
    iconBtn->setVisible(visible);
}
