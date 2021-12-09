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

const QString MENU_FILE_FILE_NEW { WindowKeeper::tr("New Document") };
const QString MENU_FILE_FOLDER_NEW { WindowKeeper::tr("New Folder") };
const QString MENU_FILE_OPEN_FILE { WindowKeeper::tr("Open Document") };
const QString MENU_FILE_OPEN_DIR { WindowKeeper::tr("Open Folder") };
const QString MENU_FILE_QUIT { WindowKeeper::tr("Quit") };
const QString OPEN_RECENT_DOCUMENTS { WindowKeeper::tr("Open Recent Documents") };
const QString OPEN_RECENT_FOLDER { WindowKeeper::tr("Open Recent Folders") };

const QString DIALOG_OPEN_DOCUMENT_TITLE { WindowKeeper::tr("Open Document") };
const QString DIALOG_OPEN_FOLDER_TITLE { WindowKeeper::tr("Open Folder") };

using namespace dpfservice;

void WindowKeeper::createFileActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    QMenu* fileMenu = new QMenu();
    QAction* actionQuit = new QAction(MENU_FILE_QUIT);
    QAction::connect(actionQuit, &QAction::triggered, [](){
        qApp->closeAllWindows();
    });

    QAction* actionNewDocument = new QAction(MENU_FILE_FILE_NEW);
    QAction::connect(actionNewDocument, &QAction::triggered, [=](){
        qInfo() << "nothing to do";
    });

    QAction* actionNewFolder = new QAction(MENU_FILE_FOLDER_NEW);
    QAction::connect(actionNewFolder, &QAction::triggered, [=](){
        qInfo() << "nothing to do";
    });

    QAction* actionOpenDocument = new QAction(MENU_FILE_OPEN_FILE);
    QAction::connect(actionOpenDocument, &QAction::triggered, [=](){
        QString file = QFileDialog::getOpenFileName(nullptr, DIALOG_OPEN_DOCUMENT_TITLE);
        if (file.isEmpty()) return;
        SendEvents::menuOpenFile(file);
    });

    QAction* actionOpenFolder = new QAction(MENU_FILE_OPEN_DIR);
    QAction::connect(actionOpenFolder, &QAction::triggered, [=](){
        QString directory = QFileDialog::getExistingDirectory(nullptr, DIALOG_OPEN_FOLDER_TITLE);
        if (directory.isEmpty()) return;
        SendEvents::menuOpenDirectory(directory);
    });

    auto openRecentDocuments = new QAction(OPEN_RECENT_DOCUMENTS);
    auto openRecentFolders = new QAction(OPEN_RECENT_FOLDER);

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
    fileAction->setText(QString::fromStdString(MENU_FILE));
}

void WindowKeeper::createBuildActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    QMenu* buildMenu = new QMenu();
    QAction* buildAction = menuBar->addMenu(buildMenu);
    buildAction->setText(QString::fromStdString(MENU_BUILD));
}

void WindowKeeper::createDebugActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    QAction* debugAction = menuBar->addMenu(new QMenu());
    debugAction->setText(QString::fromStdString(MENU_DEBUG));
}

void WindowKeeper::createToolsActions(QMenuBar *menuBar)
{
    qInfo() << __FUNCTION__;
    auto toolsMenu = new QMenu(QString::fromStdString(MENU_TOOLS));
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
    auto helpMenu = new QMenu(QString::fromStdString(MENU_HELP));
    menuBar->addMenu(helpMenu);

    QAction* actionReportBug = new QAction("Report Bug");
    helpMenu->addAction(actionReportBug);

    QAction* actionAboutUnionCode = new QAction("About \"Union Code\"");
    helpMenu->addAction(actionAboutUnionCode);
}

void WindowKeeper::createStatusBar(QMainWindow *window)
{
    qInfo() << __FUNCTION__;
    QStatusBar* statusBar = new QStatusBar();
    window->setStatusBar(statusBar);
}

void WindowKeeper::createNavRecent(QToolBar *toolbar)
{
    qInfo() << __FUNCTION__;
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
    qInfo() << __FUNCTION__;
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
    qInfo() << __FUNCTION__;
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
    qInfo() << __FUNCTION__;
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
    qInfo() << __FUNCTION__;
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
    :QObject (parent)
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
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

    QObject::connect(windowService, QOverload<const QString &, AbstractAction *>
                     ::of(&WindowService::addAction),
                     this, QOverload<const QString &, AbstractAction *>
                     ::of(&WindowKeeper::addAction));
}

void WindowKeeper::addNavAction(AbstractAction *action)
{
    Q_UNUSED(action);
    qInfo() << __FUNCTION__;
}

void WindowKeeper::addCentral(const QString &navName, AbstractCentral *central)
{
    qInfo() << __FUNCTION__;
    QWidget* inputWidget = static_cast<QWidget*>(central->qWidget());
    if(!central || !inputWidget || navName.isEmpty())
        return;

    if (centrals.values().contains(inputWidget))
        return;

    centrals.insert(navName, inputWidget);
}

void WindowKeeper::addMenu(AbstractMenu *menu)
{
    qInfo() << __FUNCTION__;

    QMenu *inputMenu = static_cast<QMenu*>(menu->qMenu());
    if (!window || !inputMenu)
        return;

    //始终将Helper置末
    for (QAction *action : window->menuBar()->actions()) {
        if (action->text() == QString::fromStdString(MENU_HELP)) {
            window->menuBar()->insertMenu(action, inputMenu);
            return; //提前返回
        }
    }

    //直接添加到最后
    window->menuBar()->addMenu(inputMenu);
}

void WindowKeeper::addAction(const QString &menuName, AbstractAction *action)
{
    qInfo() << __FUNCTION__;
    QAction *inputAction = static_cast<QAction*>(action->qAction());
    if (!action || !inputAction)
        return;

    for (QAction *qaction : window->menuBar()->actions()) {
        if (qaction->text() == menuName) {
            if (qaction->text() == QString::fromStdString(MENU_FILE)) {
                auto endAction = *(qaction->menu()->actions().rbegin());
                if (endAction->text() == MENU_FILE_QUIT) {
                    return qaction->menu()->insertAction(endAction, inputAction);
                }
            }
            qaction->menu()->addAction(inputAction);
        }
    }
}
