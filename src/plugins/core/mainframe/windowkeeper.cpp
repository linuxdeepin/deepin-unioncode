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
#include "windowkeeper.h"
#include "transceiver/sendevents.h"
#include "windowstatusbar.h"
#include "services/window/windowservice.h"
#include "common/common.h"

#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QMainWindow>
#include <QFileDialog>
#include <QApplication>
#include <QActionGroup>

static WindowKeeper *ins{nullptr};
using namespace dpfservice;
class WindowKeeperPrivate
{
    friend class WindowKeeper;
    QHash<QString, QWidget *> centrals{};
    QMainWindow *window{nullptr};
    QActionGroup *navActionGroup{nullptr};
    QToolBar *toolbar{nullptr};
};

void WindowKeeper::createFileActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    QMenu* fileMenu = new QMenu();
    QAction* actionQuit = new QAction(MWMFA_QUIT);
    QAction::connect(actionQuit, &QAction::triggered, [](){
        qApp->closeAllWindows();
    });

    QAction* actionNewDocument = new QAction(MWMFA_DOCUMENT_NEW);
    QAction::connect(actionNewDocument, &QAction::triggered, [=](){
        qInfo() << "nothing to do";
    });

    QAction* actionNewFolder = new QAction(MWMFA_FOLDER_NEW);
    QAction::connect(actionNewFolder, &QAction::triggered, [=](){
        qInfo() << "nothing to do";
    });

    QAction* actionOpenDocument = new QAction(MWMFA_OPEN_DOCUMENT);
    QAction::connect(actionOpenDocument, &QAction::triggered, [=](){
        QString file = QFileDialog::getOpenFileName(nullptr, DIALOG_OPEN_DOCUMENT_TITLE);
        if (file.isEmpty()) return;
        SendEvents::menuOpenFile(file);
    });

    QAction* actionOpenFolder = new QAction(MWMFA_OPEN_FOLDER);
    QAction::connect(actionOpenFolder, &QAction::triggered, [=](){
        QString directory = QFileDialog::getExistingDirectory(nullptr, DIALOG_OPEN_FOLDER_TITLE);
        if (directory.isEmpty()) return;
        SendEvents::menuOpenDirectory(directory);
    });

    auto openRecentDocuments = new QAction(MWMFA_OPEN_RECENT_DOCUMENTS);
    auto openRecentFolders = new QAction(MWMFA_OPEN_RECENT_FOLDER);

    fileMenu->addAction(actionNewDocument);
    fileMenu->addAction(actionNewFolder);
    fileMenu->addSeparator();
    fileMenu->addAction(actionOpenDocument);
    fileMenu->addAction(actionOpenFolder);
    fileMenu->addAction(openRecentDocuments);
    fileMenu->addAction(openRecentFolders);
    fileMenu->addSeparator();
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

    QAction* actionSearch = new QAction("Search");
    QAction* actionPackageTools = new QAction("Package Tools");
    QAction* actionVersionTools = new QAction("Version Tools");
    QAction* actionCodeFormatting = new QAction("Code Formatting");
    QAction* actionRuntimeAnalysis = new QAction("Runtime Analysis");
    QAction* actionTest = new QAction("Test");
    QAction* actionPlugins = new QAction("Plugins");
    QAction* actionOptions = new QAction("Options");
    toolsMenu->addAction(actionSearch);
    toolsMenu->addAction(actionPackageTools);
    toolsMenu->addAction(actionVersionTools);
    toolsMenu->addAction(actionCodeFormatting);
    toolsMenu->addAction(actionRuntimeAnalysis);
    toolsMenu->addAction(actionTest);
    toolsMenu->addAction(actionPlugins);
    toolsMenu->addAction(actionOptions);
}

void WindowKeeper::createHelpActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    auto helpMenu = new QMenu(MWM_HELP);
    menuBar->addMenu(helpMenu);

    QAction* actionReportBug = new QAction("Report Bug");
    helpMenu->addAction(actionReportBug);

    QAction* actionAboutUnionCode = new QAction("About \"Union Code\"");
    helpMenu->addAction(actionAboutUnionCode);
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
        WindowKeeper::switchNavWidget(MWNA_RECENT);
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
        WindowKeeper::switchNavWidget(MWNA_EDIT);
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
        WindowKeeper::switchNavWidget(MWNA_DEBUG);
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
        WindowKeeper::switchNavWidget(MWNA_RUNTIME);
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
    createNavDebug(d->toolbar);
    createNavRuntime(d->toolbar);

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
        d->window->setWindowTitle("Union Code");
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

    if (!windowService->addCentral) {
        windowService->addCentral = std::bind(&WindowKeeper::addCentral, this, _1, _2);
    }

    if (!windowService->addNavAction) {
        windowService->addNavAction = std::bind(&WindowKeeper::addNavAction, this, _1);
    }

    if (!windowService->addAction) {
        windowService->addAction = std::bind(&WindowKeeper::addAction, this, _1, _2);
    }
}

WindowKeeper::~WindowKeeper()
{
    if (d) {
        delete d;
    }
}

void WindowKeeper::addNavAction(AbstractAction *action)
{
    if (!action || !action->qAction() || !d->toolbar || !d->navActionGroup)
        return;

    auto qAction = (QAction*)action->qAction();
    qAction->setCheckable(true);
    d->navActionGroup->addAction(qAction);
    d->toolbar->addAction(qAction);
    QObject::connect(qAction, &QAction::triggered,[=](){
        switchNavWidget(qAction->text());
    });
}

void WindowKeeper::addCentral(const QString &navName, AbstractCentral *central)
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

void WindowKeeper::addAction(const QString &menuName, AbstractAction *action)
{
    qInfo() << __FUNCTION__;
    QAction *inputAction = static_cast<QAction*>(action->qAction());
    if (!action || !inputAction)
        return;

    for (QAction *qaction : d->window->menuBar()->actions()) {
        if (qaction->text() == menuName) {
            if (qaction->text() == MWM_FILE) {
                auto endAction = *(qaction->menu()->actions().rbegin());
                if (endAction->text() == MWMFA_QUIT) {
                    return qaction->menu()->insertAction(endAction, inputAction);
                }
            }
            qaction->menu()->addAction(inputAction);
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

void WindowKeeper::switchNavWidget(const QString &navName)
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
