// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controller.h"
#include "loadingwidget.h"
#include "navigationbar.h"
#include "plugindialog.h"
#include "windowstatusbar.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

#include <DFrame>
#include <DFileDialog>
#include <DTitlebar>
#include <DStackedWidget>

#include <QDebug>
#include <QShortcut>
#include <QDesktopServices>
#include <QMenuBar>
#include <QDesktopWidget>
#include <QScreen>

static Controller *ins { nullptr };
inline const QString contextWidgetName = "ContextWidget";
// MW = MainWindow
inline constexpr int MW_WIDTH { 1280 };
inline constexpr int MW_HEIGHT { 860 };

inline constexpr int MW_MIN_WIDTH { 1280 };
inline constexpr int MW_MIN_HEIGHT { 600 };
using namespace dpfservice;

class ControllerPrivate
{
    MainWindow *mainWindow { nullptr };
    loadingWidget *loadingwidget { nullptr };
    NavigationBar *navigationBar { nullptr };
    QVBoxLayout *leftBarBottomLayout { nullptr };
    QVBoxLayout *leftBarTopLayout { nullptr };

    QMap<QString, QAction *> navigationActions;

    QMap<QString, DWidget *> contextWidgets;
    QMap<QString, DPushButton *> tabButtons;
    DWidget *contextWidget { nullptr };
    DStackedWidget *stackContextWidget { nullptr };
    DFrame *contextTabBar { nullptr };

    WindowStatusBar *statusBar { nullptr };

    DMenu *menu { nullptr };

    QMap<QString, View *> viewList;
    QString currentPlugin { "" };

    friend class Controller;
};

Controller *Controller::instance()
{
    if (!ins)
        ins = new Controller;
    return ins;
}

Controller::Controller(QObject *parent)
    : QObject(parent), d(new ControllerPrivate)
{
    qInfo() << __FUNCTION__;
    initMainWindow();
    initNavigationBar();
    initContextWidget();
    initStatusBar();

    registerService();
}

void Controller::registerService()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    using namespace std::placeholders;
    //        if (!windowService->addWidget) {
    //            windowService->addWidget = std::bind(&Controller::addWidget, this, _1, _2, _3, _4);
    //        }
    //        if (!windowService->addNavigation2) {
    //            windowService->addNavigation2 = std::bind(&Controller::addNavigation, this, _1);
    //        }
    //        if (!windowService->addContextWidget2) {
    //            windowService->addContextWidget2 = std::bind(&Controller::addContextWidget, this, _1, _2, _3);
    //        }
}

Controller::~Controller()
{
    if (d->mainWindow)
        delete d->mainWindow;
    foreach (auto view, d->viewList.values()) {
        delete view;
    }
    if (d)
        delete d;
}

void Controller::raiseView(const QString &plugin)
{
    auto *view = d->viewList[plugin];
    if (!view) {
        qWarning() << "no existed View named " << plugin;
        return;
    }

    if (view->previousView.isEmpty() && (view->previousView != plugin))
        raiseView(view->previousView);

    foreach (auto hidepos, view->hiddenposList)
        d->mainWindow->hideWidget(hidepos);

    foreach (auto widgetName, view->widgetList)
        d->mainWindow->showWidget(widgetName);

    d->mainWindow->hideTopTollBar();
    foreach (auto item, view->topToolItemList)
        d->mainWindow->showTopToolItem(item);

    if (view->showContextWidget && !d->contextWidgets.isEmpty())
        d->mainWindow->showWidget(contextWidgetName);

    d->currentPlugin = plugin;
}

void Controller::setCurrentPlugin(const QString &plugin)
{
    d->currentPlugin = plugin;
}

void Controller::setPreviousView(const QString &pluginName, const QString &previous)
{
    if (!d->viewList.contains(pluginName) || !d->viewList.contains(previous)) {
        qWarning() << pluginName << "or" << previous << "haven`t set yet";
        return;
    }
    d->viewList[pluginName]->previousView = previous;
}

void Controller::addWidget(const QString &name, AbstractWidget *abstractWidget, Position pos, bool replace)
{
    auto widget = static_cast<DWidget *>(abstractWidget->qWidget());

    auto view = d->viewList[d->currentPlugin];
    if (!view) {
        view = new View;
        view->pluginName = d->currentPlugin;
        view->widgetList.append(name);
        d->viewList.insert(d->currentPlugin, view);
    } else {
        raiseView(d->currentPlugin);
        return;
    }

    if (replace) {
        view->hiddenposList.append(pos);
        d->mainWindow->hideWidget(pos);
    }

    d->mainWindow->addWidget(name, widget, pos);
}

void Controller::addNavigation(AbstractAction *action)
{
    if (!action)
        return;
    auto inputAction = static_cast<QAction *>(action->qAction());

    d->navigationBar->addNavItem(inputAction);
    d->navigationActions.insert(inputAction->text(), inputAction);
}

void Controller::addNavigationToBottom(AbstractAction *action)
{
    if (!action)
        return;
    auto inputAction = static_cast<QAction *>(action->qAction());

    d->navigationBar->addNavItem(inputAction, NavigationBar::bottom);
    d->navigationActions.insert(inputAction->text(), inputAction);
}

void Controller::switchWidgetNavigation(const QString &navName)
{
    if (navName != d->currentPlugin)
        d->navigationBar->setNavActionChecked(navName, true);

    if(hasView(navName)) {
        d->currentPlugin = navName;
        raiseView(navName);
    }
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
        swtichContextWidget(title);
    });

    d->tabButtons.insert(title, tabBtn);
}

void Controller::showContextWidget()
{
    d->mainWindow->showWidget(contextWidgetName);
    d->viewList[d->currentPlugin]->showContextWidget = true;
}

bool Controller::hasContextWidget(const QString &title)
{
    return d->contextWidgets.contains(title);
}

void Controller::hideContextWidget()
{
    d->mainWindow->hideWidget(contextWidgetName);
    d->viewList[d->currentPlugin]->showContextWidget = false;
}

void Controller::switchContextWidget(const QString &title)
{
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
    DMenu *inputMenu = static_cast<DMenu *>(abstractMenu->qMenu());
    if (!d->mainWindow || !inputMenu)
        return;

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
    QAction *inputAction = static_cast<QAction *>(action->qAction());
    if (!action || !inputAction)
        return;

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
    QAction *inputAction = static_cast<QAction *>(action->qAction());
    if (!inputAction)
        return;

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

    QAction *inputAction = static_cast<QAction *>(action->qAction());

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

void Controller::addTopToolItem(const QString &name, AbstractAction *action)
{
    if (!action || name.isNull())
        return;
    auto inputAction = static_cast<QAction *>(action->qAction());

    auto view = d->viewList[d->currentPlugin];
    if (!view->topToolItemList.contains(name)) {
        view->topToolItemList.append(name);
        d->mainWindow->addTopToolItem(name, inputAction);
    }
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

void Controller::showAboutPlugins()
{
    PluginDialog dialog;
    dialog.exec();
}

void Controller::loading()
{
    d->loadingwidget = new loadingWidget(d->mainWindow);
    d->mainWindow->addWidget("loadingWidget", d->loadingwidget);

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                     this, [=]() {
                         //d->navigationActions[MWNA_RECENT]->trigger();
                         d->mainWindow->removeWidget("loadingWidget");

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
    auto helpMenu = new DMenu(MWM_HELP);
    d->menu->addMenu(helpMenu);

    QAction *actionReportBug = new QAction(MWM_REPORT_BUG);
    ActionManager::getInstance()->registerAction(actionReportBug, "Help.Report.Bug",
                                                 MWM_REPORT_BUG, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_R),
                                                 "");
    addMenuShortCut(actionReportBug);
    helpMenu->addAction(actionReportBug);
    QAction *actionHelpDoc = new QAction(MWM_HELP_DOCUMENTS);
    ActionManager::getInstance()->registerAction(actionHelpDoc, "Help.Help.Documents",
                                                 MWM_HELP_DOCUMENTS, QKeySequence());
    helpMenu->addAction(actionHelpDoc);

    helpMenu->addSeparator();

    QAction *actionAboutPlugin = new QAction(MWM_ABOUT_PLUGINS);
    ActionManager::getInstance()->registerAction(actionAboutPlugin, "Help.AboutPlugins", MWM_ABOUT_PLUGINS, QKeySequence());
    helpMenu->addAction(actionAboutPlugin);

    QAction::connect(actionReportBug, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://github.com/linuxdeepin/deepin-unioncode/issues"));
    });
    QAction::connect(actionHelpDoc, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(QUrl("https://ecology.chinauos.com/adaptidentification/doc_new/#document2?dirid=656d40a9bd766615b0b02e5e"));
    });
    QAction::connect(actionAboutPlugin, &QAction::triggered, this, &Controller::showAboutPlugins);
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
    contextVLayout->addWidget(new DHorizontalLine);
    contextVLayout->addWidget(d->contextTabBar);
    contextVLayout->addWidget(new DHorizontalLine);
    contextVLayout->addWidget(d->stackContextWidget);
    d->contextWidget->setLayout(contextVLayout);

    d->mainWindow->addWidget(contextWidgetName, d->contextWidget, Position::Bottom);
    d->mainWindow->hideWidget(contextWidgetName);
}

void Controller::initStatusBar()
{
    if (d->statusBar)
        return;
    d->statusBar = new WindowStatusBar(d->mainWindow);
    d->statusBar->hide();
    d->mainWindow->setStatusBar(d->statusBar);
}

void Controller::addMenuShortCut(QAction *action, QKeySequence keySequence)
{
    QKeySequence key = keySequence;
    if (keySequence.isEmpty())
        key = action->shortcut();

    QShortcut *shortCutOpenFile = new QShortcut(key, d->mainWindow);
    connect(shortCutOpenFile, &QShortcut::activated, [=] {
        action->trigger();
    });
}
