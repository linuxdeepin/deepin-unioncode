#include "windowkeeper.h"
#include "sendevents.h"
#include "services/window/windowservice.h"

#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QMainWindow>
#include <QFileDialog>
#include <QApplication>

QHash<QString, QWidget *> centrals{};
QMainWindow *window = nullptr;
const int minWidth = 1200;
const int minHeight = 800;

const std::string MENU_FILE_PROJECT_NEW { WindowKeeper::tr("New File/Directory").toStdString() };
const std::string MENU_FILE_OPEN_FILE { WindowKeeper::tr("Open File").toStdString() };
const std::string MENU_FILE_OPEN_DIR { WindowKeeper::tr("Open Directory").toStdString() };
const std::string MENU_FILE_QUIT { WindowKeeper::tr("Quit").toStdString() };
const std::string MENU_BUILD_SELECT { WindowKeeper::tr("Build Select File").toStdString() };

const QString DIALOG_OPEN_TITLE { WindowKeeper::tr("Open file or directory") };

using namespace dpfservice;

void WindowKeeper::createFileActions(QMenuBar *menuBar)
{
    QMenu* fileMenu = new QMenu();
    QAction* actionQuit = new QAction();
    actionQuit->setText(QString::fromStdString(MENU_FILE_QUIT));
    QAction::connect(actionQuit, &QAction::triggered, [](){
        qApp->closeAllWindows();
    });

    QAction* actionNewProject = new QAction();
    actionNewProject->setText(QString::fromStdString(MENU_FILE_PROJECT_NEW));
    QAction::connect(actionNewProject, &QAction::triggered, [=](){
        qInfo() << "nothing to do";
    });

    QAction* actionOpenFile = new QAction();
    actionOpenFile->setText(QString::fromStdString(MENU_FILE_OPEN_FILE));
    QAction::connect(actionOpenFile, &QAction::triggered, [=](){
        QString file = QFileDialog::getOpenFileName(nullptr, DIALOG_OPEN_TITLE);
        SendEvents::menuOpenFile(file);
    });

    QAction* actionOpenDirecoty = new QAction();
    actionOpenDirecoty->setText(QString::fromStdString(MENU_FILE_OPEN_DIR));
    QAction::connect(actionOpenDirecoty, &QAction::triggered, [=](){
        QString directory = QFileDialog::getExistingDirectory(nullptr, DIALOG_OPEN_TITLE);
        SendEvents::menuOpenDirectory(directory);
    });

    fileMenu->addAction(actionNewProject);
    fileMenu->addAction(actionOpenFile);
    fileMenu->addAction(actionOpenDirecoty);
    fileMenu->addAction(actionQuit);
    QAction* fileAction = menuBar->addMenu(fileMenu);
    fileAction->setText(QString::fromStdString(MENU_FILE));
}

void WindowKeeper::createBuildActions(QMenuBar *menuBar)
{
    QMenu* buildMenu = new QMenu();
    QAction* actionBuildSelect = new QAction();
    actionBuildSelect->setText(QString::fromStdString(MENU_BUILD_SELECT));
    QAction::connect(actionBuildSelect, &QAction::trigger, [=](){
        qInfo() << "send event from action build select";
    });
    buildMenu->addAction(actionBuildSelect);
    QAction* buildAction = menuBar->addMenu(buildMenu);
    buildAction->setText(QString::fromStdString(MENU_BUILD));
}

void WindowKeeper::createDebugActions(QMenuBar *menuBar)
{
    QAction* debugAction = menuBar->addMenu(new QMenu());
    debugAction->setText(QString::fromStdString(MENU_DEBUG));
}

void WindowKeeper::createToolsActions(QMenuBar *menuBar)
{
    QAction* toolsAction = menuBar->addMenu(new QMenu());
    toolsAction->setText(QString::fromStdString(MENU_TOOLS));
}

void WindowKeeper::createHelpActions(QMenuBar *menuBar)
{
    QAction* HelpAction = menuBar->addMenu(new QMenu());
    HelpAction->setText(QString::fromStdString(MENU_HELP));
}

void WindowKeeper::createStatusBar(QMainWindow *window)
{
    QStatusBar* statusBar = new QStatusBar();
    window->setStatusBar(statusBar);
}

void WindowKeeper::createNavRecent(QToolBar *toolbar)
{
    if (!toolbar)
        return;

    QAction* navRecent = new QAction(toolbar);
    navRecent->setText(QString::fromStdString(NAVACTION_RECENT));
    QAction::connect(navRecent, &QAction::triggered, [=](){
        SendEvents::navRecentShow(); //recent show event
    });
    toolbar->addAction(navRecent);
}

void WindowKeeper::createNavEdit(QToolBar *toolbar)
{
    if (!toolbar)
        return;

    QAction* navEdit = new QAction(toolbar);
    navEdit->setText(QString::fromStdString(NAVACTION_EDIT));
    QAction::connect(navEdit, &QAction::triggered, [=](){
        SendEvents::navEditShow();
    });

    toolbar->addAction(navEdit);
}

void WindowKeeper::createNavDebug(QToolBar *toolbar)
{
    if (!toolbar)
        return;

    QAction* navDebug = new QAction(toolbar);
    navDebug->setText(QString::fromStdString(NAVACTION_DEBUG));
    QAction::connect(navDebug, &QAction::triggered, [=](){
        SendEvents::navDebugShow();
    });

    toolbar->addAction(navDebug);
}

void WindowKeeper::createNavRuntime(QToolBar *toolbar)
{
    if (!toolbar)
        return;

    QAction *navRuntime = new QAction(toolbar);
    navRuntime->setText(QString::fromStdString(NAVACTION_RUNTIME));
    QAction::connect(navRuntime, &QAction::triggered, [=](){
        SendEvents::navRuntimeShow();
    });

    toolbar->addAction(navRuntime);
}

void WindowKeeper::layoutWindow(QMainWindow *window)
{
    QToolBar* nav = new QToolBar();
    createNavRecent(nav);
    createNavEdit(nav);
    createNavDebug(nav);
    createNavRuntime(nav);

    QMenuBar* menuBar = new QMenuBar();
    createFileActions(menuBar);
    createBuildActions(menuBar);
    createDebugActions(menuBar);
    createToolsActions(menuBar);
    createHelpActions(menuBar);

    createStatusBar(window);

    window->addToolBar(Qt::LeftToolBarArea, nav);
    window->setMinimumSize(QSize(minWidth,minHeight));
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setMenuBar(menuBar);
}

WindowKeeper::WindowKeeper(QObject *parent)
{
    auto &ctx = dpfInstance.serviceContext();
    qInfo() << "import service list" <<  ctx.services();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        windowService->name();
    }

    if (!window) {
        window = new QMainWindow();
        window->setWindowTitle("Union Code");
        layoutWindow(window);
        QMainWindow::connect(window, &QMainWindow::destroyed, [=](){
            window = nullptr;
        });
        window->show();
    }

    QObject::connect(windowService, &WindowService::addMenu,
                     this, &WindowKeeper::addMenu, Qt::UniqueConnection);

    QObject::connect(windowService, &WindowService::addCentral,
                     this, &WindowKeeper::addCentral, Qt::UniqueConnection);

    QObject::connect(windowService, &WindowService::addNavAction,
                     this, &WindowKeeper::addNavAction, Qt::UniqueConnection);

    QObject::connect(windowService, QOverload<const QString &, AbstractAction *>::of(&WindowService::addAction),
                     this, QOverload<const QString &, AbstractAction *>::of(&WindowKeeper::addAction));
}

void WindowKeeper::addNavAction(AbstractAction *action)
{
    qInfo() << __FUNCTION__;
}

void WindowKeeper::addCentral(const QString &navName, AbstractCentral *central)
{
    qInfo() << __FUNCTION__ << this;

    if(!central || !central->qWidget() || navName.isEmpty())
        return;

    QWidget *centralWidget = (QWidget*)central->qWidget();
    if (centrals.values().contains(centralWidget))
        return;

//    centralWidget->setParent(window);
    centrals.insert(navName, centralWidget);
}

void WindowKeeper::addMenu(AbstractMenu *menu)
{
    qInfo() << __FUNCTION__;

    if (!window || !window->menuBar())
        return;

    //始终将Helper置末
    for (QAction *action : window->menuBar()->actions()) {
        if (action->text() == QString::fromStdString(MENU_HELP)) {
            window->menuBar()->insertMenu(action, (QMenu*)menu->qMenu());
            return; //提前返回
        }
    }

    //直接添加到最后
    window->menuBar()->addMenu((QMenu*)menu->qMenu());
}

void WindowKeeper::addAction(const QString &menuName, AbstractAction *action)
{
    qInfo() << __FUNCTION__;

    for (QAction *qaction : window->menuBar()->actions()) {
        if (qaction->text() == menuName) {
            qaction->menu()->addAction((QAction*)action->qAction());
        }
    }
}








