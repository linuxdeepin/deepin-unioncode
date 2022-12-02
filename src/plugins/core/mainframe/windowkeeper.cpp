/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
 *             hongjinchuan<hongjinchuan@uniontech.com>
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
#include "windowkeeper.h"
#include "windowstatusbar.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "common/common.h"
#include "aboutdialog.h"

#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QMainWindow>
#include <QFileDialog>
#include <QApplication>
#include <QActionGroup>
#include <QDesktopServices>
#include <QComboBox>
#include <QTranslator>

static WindowKeeper *ins{nullptr};
using namespace dpfservice;
class WindowKeeperPrivate
{
    WindowKeeperPrivate();
    QHash<QString, QWidget *> centrals{};
    QMainWindow *window{nullptr};
    QActionGroup *navActionGroup{nullptr};
    QToolBar *toolbar{nullptr};

    friend class WindowKeeper;
};

WindowKeeperPrivate::WindowKeeperPrivate()
{

}

void WindowKeeper::createFileActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    QMenu* fileMenu = new QMenu();
    QAction* actionQuit = new QAction(MWMFA_QUIT);
    ActionManager::getInstance()->registerAction(actionQuit, "File.Quit",
                                                 MWMFA_QUIT, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_Q),
                                                 "quit.png");
    QAction::connect(actionQuit, &QAction::triggered, [](){
        qApp->closeAllWindows();
    });

    QMenu* menuOpenProject = new QMenu(MWMFA_OPEN_PROJECT);
    menuOpenProject->setIcon(QIcon(":/core/images/open_doc.png"));

    fileMenu->addMenu(menuOpenProject);
    fileMenu->addAction(actionQuit);
    QAction* fileAction = menuBar->addMenu(fileMenu);
    fileAction->setText(MWM_FILE);
}

void WindowKeeper::createBuildActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    QMenu* buildMenu = new QMenu();
    QAction* buildAction = menuBar->addMenu(buildMenu);
    buildAction->setText(MWM_BUILD);
}

void WindowKeeper::createDebugActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    QAction* debugAction = menuBar->addMenu(new QMenu());
    debugAction->setText(MWM_DEBUG);
}

void WindowKeeper::createToolsActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    auto toolsMenu = new QMenu(MWM_TOOLS);
    menuBar->addMenu(toolsMenu);

    QMenu* languageMenu = new QMenu(MWMTM_SWITCH_LANGUAGE);
    QAction* actionEnglish = new QAction(MWMTA_ENGLISH);
    QAction* actionChinese = new QAction(MWMTA_CHINESE);
    languageMenu->addAction(actionEnglish);
    languageMenu->addAction(actionChinese);
    ActionManager::getInstance()->registerAction(actionEnglish, "Tools.English", MWMTA_ENGLISH);
    ActionManager::getInstance()->registerAction(actionChinese, "Tools.Chinese", MWMTA_PLUGINS);

    toolsMenu->addMenu(languageMenu);

    actionEnglish->setEnabled(false);
    actionChinese->setEnabled(false);
}

void WindowKeeper::createHelpActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    auto helpMenu = new QMenu(MWM_HELP);
    menuBar->addMenu(helpMenu);

    QAction* actionReportBug = new QAction(MWM_REPORT_BUG);
    ActionManager::getInstance()->registerAction(actionReportBug, "Help.Report.Bug",
                                                 MWM_REPORT_BUG, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_R),
                                                 "tools-report-bug.png");
    helpMenu->addAction(actionReportBug);

    QAction* actionAboutUnionCode = new QAction(MWM_ABOUT);
    ActionManager::getInstance()->registerAction(actionAboutUnionCode, "Help.About",
                                                 MWM_ABOUT, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_A),
                                                 "help-about.svg");
    helpMenu->addAction(actionAboutUnionCode);

    QAction::connect(actionAboutUnionCode, &QAction::triggered, this, &WindowKeeper::showAboutDlg);
    QAction::connect(actionReportBug, &QAction::triggered, [=](){
        QDesktopServices::openUrl(QUrl("https://pms.uniontech.com/project-bug-1039.html"));
    });
}

void WindowKeeper::createStatusBar(QMainWindow *window)
{
    qInfo() << __FUNCTION__;
    QStatusBar* statusBar = new WindowStatusBar();
    window->setStatusBar(statusBar);
}

void WindowKeeper::createNavRecent(QToolBar *toolbar)
{
    qInfo() << __FUNCTION__;
    if (!toolbar)
        return;

    QAction* navRecent = new QAction(MWNA_RECENT, toolbar);
    navRecent->setCheckable(true);
    d->navActionGroup->addAction(navRecent);
    QAction::connect(navRecent, &QAction::triggered, [=](){
        WindowKeeper::switchWidgetNavigation(MWNA_RECENT);
    });
    toolbar->addAction(navRecent);
}

void WindowKeeper::createNavEdit(QToolBar *toolbar)
{
    qInfo() << __FUNCTION__;
    if (!toolbar)
        return;

    QAction* navEdit = new QAction(toolbar);
    navEdit->setCheckable(true);
    d->navActionGroup->addAction(navEdit);
    navEdit->setText(MWNA_EDIT);
    QAction::connect(navEdit, &QAction::triggered, [=](){
        WindowKeeper::switchWidgetNavigation(MWNA_EDIT);
    });

    toolbar->addAction(navEdit);
}

void WindowKeeper::createNavDebug(QToolBar *toolbar)
{
    qInfo() << __FUNCTION__;
    if (!toolbar)
        return;

    QAction* navDebug = new QAction(toolbar);
    navDebug->setCheckable(true);
    d->navActionGroup->addAction(navDebug);
    navDebug->setText(MWNA_DEBUG);
    QAction::connect(navDebug, &QAction::triggered, [=](){
        WindowKeeper::switchWidgetNavigation(MWNA_DEBUG);
    });

    toolbar->addAction(navDebug);
}

void WindowKeeper::createNavRuntime(QToolBar *toolbar)
{
    qInfo() << __FUNCTION__;
    if (!toolbar)
        return;

    QAction *navRuntime = new QAction(toolbar);
    navRuntime->setCheckable(true);
    d->navActionGroup->addAction(navRuntime);
    navRuntime->setText(MWNA_RUNTIME);
    QAction::connect(navRuntime, &QAction::triggered, [=](){
        WindowKeeper::switchWidgetNavigation(MWNA_RUNTIME);
    });

    toolbar->addAction(navRuntime);
}

void WindowKeeper::layoutWindow(QMainWindow *window)
{
    qInfo() << __FUNCTION__;
    if (!d->navActionGroup)
        d->navActionGroup = new QActionGroup(window);

    d->toolbar = new QToolBar(QToolBar::tr("Navigation"));
    createNavRecent(d->toolbar);
    createNavEdit(d->toolbar);
    //createNavDebug(d->toolbar);
    //createNavRuntime(d->toolbar);

    QMenuBar* menuBar = new QMenuBar();
    createFileActions(menuBar);
    createBuildActions(menuBar);
    createDebugActions(menuBar);
    createToolsActions(menuBar);
    createHelpActions(menuBar);

    createStatusBar(window);

    window->addToolBar(Qt::LeftToolBarArea, d->toolbar);
    window->setMinimumSize(QSize(MW_MIN_WIDTH,MW_MIN_HEIGHT));
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setMenuBar(menuBar);
}

WindowKeeper *WindowKeeper::instace()
{
    if (!ins)
        ins = new WindowKeeper;
    return ins;
}

WindowKeeper::WindowKeeper(QObject *parent)
    : QObject (parent)
    , d (new WindowKeeperPrivate)
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        windowService->name();
    }

    if (!d->window) {
        d->window = new QMainWindow();
        d->window->setWindowTitle("Deepin Union Code");
        d->window->setWindowIcon(QIcon(":/core/images/unioncode@128.png"));
        QObject::connect(d->window, &QMainWindow::destroyed, [&](){
            d->window->takeCentralWidget();
        });
        layoutWindow(d->window);
        d->window->show();
    }

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted,
                     this, &WindowKeeper::initUserWidget);

    using namespace std::placeholders;
    if (!windowService->addMenu) {
        windowService->addMenu = std::bind(&WindowKeeper::addMenu, this, _1);
    }

    if (!windowService->addCentralNavigation) {
        windowService->addCentralNavigation = std::bind(&WindowKeeper::addCentralNavigation, this, _1, _2);
    }

    if (!windowService->addActionNavigation) {
        windowService->addActionNavigation = std::bind(&WindowKeeper::addActionNavigation, this, _1);
    }

    if (!windowService->addAction) {
        windowService->addAction = std::bind(&WindowKeeper::addAction, this, _1, _2);
    }

    if (!windowService->removeActions) {
        windowService->removeActions = std::bind(&WindowKeeper::removeActions, this, _1);
    }
}

WindowKeeper::~WindowKeeper()
{
    if (d) {
        delete d;
    }
}

QStringList WindowKeeper::navActionTexts() const
{
    return d->centrals.keys();
}

void WindowKeeper::addActionNavigation(AbstractAction *action)
{
    if (!action || !action->qAction() || !d->toolbar || !d->navActionGroup)
        return;

    auto qAction = (QAction*)action->qAction();
    qAction->setCheckable(true);
    d->navActionGroup->addAction(qAction);
    d->toolbar->addAction(qAction);
    QObject::connect(qAction, &QAction::triggered,[=](){
        switchWidgetNavigation(qAction->text());
    });
}

void WindowKeeper::addCentralNavigation(const QString &navName, AbstractCentral *central)
{
    qInfo() << __FUNCTION__;
    QWidget* inputWidget = static_cast<QWidget*>(central->qWidget());
    if(!central || !inputWidget || navName.isEmpty())
        return;

    if (d->centrals.values().contains(inputWidget))
        return;

    inputWidget->setParent(d->window);
    d->centrals.insert(navName, inputWidget);
}

void WindowKeeper::addMenu(AbstractMenu *menu)
{
    qInfo() << __FUNCTION__;

    QMenu *inputMenu = static_cast<QMenu*>(menu->qMenu());
    if (!d->window || !inputMenu)
        return;

    //始终将Helper置末
    for (QAction *action : d->window->menuBar()->actions()) {
        if (action->text() == MWM_HELP) {
            d->window->menuBar()->insertMenu(action, inputMenu);
            return; //提前返回
        }
    }

    //直接添加到最后
    d->window->menuBar()->addMenu(inputMenu);
}

void WindowKeeper::insertAction(const QString &menuName,
                                const QString &beforActionName,
                                AbstractAction *action)
{
    qInfo() << __FUNCTION__;
    QAction *inputAction = static_cast<QAction*>(action->qAction());
    if (!action || !inputAction)
        return;

    for (QAction *qAction : d->window->menuBar()->actions()) {
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

void WindowKeeper::addAction(const QString &menuName, AbstractAction *action)
{
    qInfo() << __FUNCTION__;
    QAction *inputAction = static_cast<QAction*>(action->qAction());
    if (!action || !inputAction)
        return;

    for (QAction *qAction : d->window->menuBar()->actions()) {
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

void WindowKeeper::removeActions(const QString &menuName)
{
    qInfo() << __FUNCTION__;
    for (QAction *qAction : d->window->menuBar()->actions()) {
        if (qAction->text() == menuName) {
            foreach (QAction *action, qAction->menu()->actions()) {
                qAction->menu()->removeAction(action);
            }

            break;
        }
    }
}

void WindowKeeper::addOpenProjectAction(const QString &name, AbstractAction *action)
{
    qInfo() << __FUNCTION__;
    if (!action || !action->qAction())
        return;

    QAction *inputAction = static_cast<QAction*>(action->qAction());
    for (QAction *qAction : d->window->menuBar()->actions()) {
        if (qAction->text() == MWM_FILE) {
            for(QAction *childAction : qAction->menu()->actions()) {
                if (childAction->text() == MWMFA_OPEN_PROJECT) {
                    for (auto langAction : childAction->menu()->menuAction()->menu()->actions()) {
                        if (name == langAction->text()) {
                            langAction->menu()->addAction(inputAction);
                            return;
                        }
                    }
                    auto langMenu = new QMenu(name);
                    childAction->menu()->addMenu(langMenu);
                    langMenu->addAction(inputAction);
                    return;
                }
            }
        }
    }
}

void WindowKeeper::initUserWidget()
{
    qApp->processEvents();
    if (d->toolbar->actions().size() > 0) {
        d->toolbar->actions().at(0)->trigger();
    }
}

void WindowKeeper::switchWidgetNavigation(const QString &navName)
{
    auto beforWidget = d->window->takeCentralWidget();
    if (beforWidget)
        beforWidget->hide();

    setNavActionChecked(navName, true);

    if (d->centrals.isEmpty() || !d->window)
        return;

    auto widget = d->centrals[navName];
    if (!widget)
        return;

    d->window->setCentralWidget(widget);
    d->window->centralWidget()->show();
}

void WindowKeeper::setNavActionChecked(const QString &actionName, bool checked)
{
    if (!d->navActionGroup)
        return;

    for (auto action : d->navActionGroup->actions()) {
        if (action->text() == actionName){
            qInfo() << action->text();
            action->setChecked(checked);
        }
    }
}

void WindowKeeper::showAboutDlg()
{
    AboutDialog dlg;
    dlg.exec();
}
