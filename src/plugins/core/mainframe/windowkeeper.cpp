// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aboutdialog.h"
#include "plugindialog.h"

#include "windowkeeper.h"
#include "windowstatusbar.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "common/common.h"

#include <DWidget>
#include <DToolBar>
#include <DMenu>
#include <DTitlebar>
#include <DStatusBar>
#include <DFileDialog>
#include <DWidget>
#include <DFrame>
#include <DToolButton>

#include <QDesktopServices>
#include <QVBoxLayout>
#include <QAction>
#include <QActionGroup>
#include <QDesktopWidget>
#include <QComboBox>
#include <QTranslator>
#include <QScreen>
#include <QStandardPaths>

static WindowKeeper *ins{nullptr};
using namespace dpfservice;
class WindowKeeperPrivate
{
    WindowKeeperPrivate();
    QHash<QString, DWidget *> centrals{};
    DMainWindow *window{nullptr};
    QActionGroup *navActionGroup{nullptr};
    DMenu *mainMenu{nullptr};
    DToolBar *editToolBar{nullptr};

    DFrame *leftToolBar{nullptr};
    DWidget *topToolBar{nullptr};
    QHash<QString, DToolButton*> leftToolBtns;
    QList<DIconButton*> topToolBtnList;

    friend class WindowKeeper;
};

WindowKeeperPrivate::WindowKeeperPrivate()
{

}

void WindowKeeper::createFileActions(DMenu *menu)
{
    qInfo() << __FUNCTION__;
    QAction* actionOpenFile = new QAction(MWMFA_OPEN_FILE);
    ActionManager::getInstance()->registerAction(actionOpenFile, "File.Open.File",
                                                 MWMFA_OPEN_FILE, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_O));

    QAction::connect(actionOpenFile, &QAction::triggered, [=](){
        QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        QString filePath = DFileDialog::getOpenFileName(nullptr, DIALOG_OPEN_DOCUMENT_TITLE, dir);
        if (filePath.isEmpty() && !QFileInfo(filePath).exists())
            return;
        recent.saveOpenedFile(filePath);
        editor.openFile(filePath);
    });

    menu->addAction(actionOpenFile);

    DMenu* menuOpenProject = new DMenu(MWMFA_OPEN_PROJECT);
    menu->addMenu(menuOpenProject);
}

void WindowKeeper::createBuildActions(DMenu *menu)
{
    qInfo() << __FUNCTION__;
    DMenu* buildMenu = new DMenu();
    QAction* buildAction = menu->addMenu(buildMenu);
    buildAction->setText(MWM_BUILD);
}

void WindowKeeper::createDebugActions(DMenu *menu)
{
    qInfo() << __FUNCTION__;
    QAction* debugAction = menu->addMenu(new DMenu());
    debugAction->setText(MWM_DEBUG);
}

void WindowKeeper::createToolsActions(DMenu *menu)
{
    qInfo() << __FUNCTION__;
    auto toolsMenu = new DMenu(MWM_TOOLS);
    menu->addMenu(toolsMenu);
}

void WindowKeeper::createHelpActions(DMenu *menu)
{
    qInfo() << __FUNCTION__;
    auto helpMenu = new DMenu(MWM_HELP);
    menu->addMenu(helpMenu);

    QAction *actionReportBug = new QAction(MWM_REPORT_BUG);
    ActionManager::getInstance()->registerAction(actionReportBug, "Help.Report.Bug",
                                                 MWM_REPORT_BUG, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_R),
                                                 ":/core/images/tools-report-bug.png");
    helpMenu->addAction(actionReportBug);
    QAction *actionHelpDoc = new QAction(MWM_HELP_DOCUMENTS);
    ActionManager::getInstance()->registerAction(actionHelpDoc, "Help.Help.Documents",
                                                 MWM_HELP_DOCUMENTS, QKeySequence());
    helpMenu->addAction(actionHelpDoc);

    helpMenu->addSeparator();

    QAction *actionAboutUnionCode = new QAction(MWM_ABOUT);
    ActionManager::getInstance()->registerAction(actionAboutUnionCode, "Help.About",
                                                 MWM_ABOUT, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_A),
                                                 ":/core/images/help-about.svg");
    helpMenu->addAction(actionAboutUnionCode);

    QAction *actionAboutPlugin = new QAction(MWM_ABOUT_PLUGINS);
    ActionManager::getInstance()->registerAction(actionAboutUnionCode, "Help.AboutPlugins", MWM_ABOUT, QKeySequence());
    helpMenu->addAction(actionAboutPlugin);

    QAction::connect(actionReportBug, &QAction::triggered, [=](){
        QDesktopServices::openUrl(QUrl("https://pms.uniontech.com/project-bug-1039.html"));
    });
    QAction::connect(actionHelpDoc, &QAction::triggered, [=](){
        QDesktopServices::openUrl(QUrl("https://wiki.deepin.org/zh/05_HOW-TO/02_%E5%BC%80%E5%8F%91%E7%9B%B8%E5%85%B3/deepin-unioncode"));
    });
    QAction::connect(actionAboutUnionCode, &QAction::triggered, this, &WindowKeeper::showAboutDlg);
    QAction::connect(actionAboutPlugin, &QAction::triggered, this, &WindowKeeper::showAboutPlugins);
}

void WindowKeeper::createStatusBar(DMainWindow *window)
{
    qInfo() << __FUNCTION__;
    DStatusBar* statusBar = new WindowStatusBar();
    window->setStatusBar(statusBar);
}

void WindowKeeper::createNavIconBtn(const QString &navName)
{
    qInfo() << __FUNCTION__;

    DToolButton *toolBtn = new DToolButton;
    toolBtn->setCheckable(true);
    toolBtn->setChecked(true);
    toolBtn->setIcon(QIcon::fromTheme(navName.toLower()));

    toolBtn->setMinimumSize(QSize(48, 48));
    toolBtn->setIconSize(QSize(20, 20));

    d->leftToolBtns.insert(navName, toolBtn);

    connect(toolBtn, &DIconButton::clicked, [=](){
        WindowKeeper::switchWidgetNavigation(navName);
    });

    QVBoxLayout *toolbarLayout = static_cast<QVBoxLayout*>(d->leftToolBar->layout());
    toolbarLayout->addSpacing(5);
    toolbarLayout->addWidget(toolBtn);
}

void WindowKeeper::createMainMenu(DMenu *menu)
{
    qInfo() << __FUNCTION__;

    if (!menu)
        return;

    createFileActions(menu);
    createBuildActions(menu);
    createDebugActions(menu);
    menu->addSeparator();

    createToolsActions(menu);
    createHelpActions(menu);
}

void WindowKeeper::initLeftToolBar()
{
    if (!d->leftToolBar)
        return;

    d->leftToolBar->setLineWidth(0);
    d->leftToolBar->setFixedWidth(58);
    DStyle::setFrameRadius(d->leftToolBar, 0);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    d->leftToolBar->setLayout(layout);
}

void WindowKeeper::layoutWindow(DMainWindow *window)
{
    qInfo() << __FUNCTION__;
    if (!d->navActionGroup)
        d->navActionGroup = new QActionGroup(window);

    createNavIconBtn(MWNA_RECENT);
    createNavIconBtn(MWNA_EDIT);

    createMainMenu(d->mainMenu);

    //createStatusBar(window);

    window->setWindowTitle("Deepin Union Code");
    window->setWindowIcon(QIcon(":/core/images/unioncode@128.png"));
    window->setMinimumSize(QSize(MW_MIN_WIDTH,MW_MIN_HEIGHT));
    window->setAttribute(Qt::WA_DeleteOnClose);

    window->titlebar()->setIcon(QIcon(":/core/images/unioncode@128.png"));
    window->titlebar()->setTitle(QString(tr("Deepin Union Code")));
    window->titlebar()->setContentsMargins(0, 0, 0, 0);

    window->titlebar()->setMenu(d->mainMenu);
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
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (!d->window) {
        d->window = new DMainWindow();

        if (!d->leftToolBar) {
            d->leftToolBar = new DFrame();
            initLeftToolBar();
        }

        if (!d->mainMenu) {
            d->mainMenu = new DMenu(d->window->titlebar());
        }

        QObject::connect(d->window, &DMainWindow::destroyed, [&](){
            d->window->takeCentralWidget();
        });
        layoutWindow(d->window);

        //CommandLine Model will not show main window
        if (CommandParser::instance().getModel() != CommandParser::CommandLine)
            d->window->show();
        int currentScreenIndex = qApp->desktop()->screenNumber(d->window);
        QList<QScreen *> screenList = QGuiApplication::screens();

        if (currentScreenIndex < screenList.count()) {
            QRect screenRect = screenList[currentScreenIndex]->geometry();
            int screenWidth = screenRect.width();
            int screenHeight = screenRect.height();
            d->window->move((screenWidth - d->window->width()) / 2, (screenHeight - d->window->height()) / 2);
        }
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

    if (!windowService->addNavigation) {
        windowService->addNavigation = std::bind(&WindowKeeper::addNavigation, this, _1);
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

void WindowKeeper::addNavigation(const QString &id)
{
    createNavIconBtn(id);
}

void WindowKeeper::addCentralNavigation(const QString &navName, AbstractCentral *central)
{
    qInfo() << __FUNCTION__;
    DWidget* inputWidget = static_cast<DWidget*>(central->qWidget());
    if(!central || !inputWidget || navName.isEmpty())
        return;

    if (d->centrals.values().contains(inputWidget))
        return;

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (navName == MWNA_EDIT) {
        d->editToolBar = static_cast<DToolBar*>(windowService->getEditToolBar());
        d->topToolBar = new DWidget();
        QHBoxLayout *layout = new QHBoxLayout();
        d->topToolBar->setLayout(layout);
        d->topToolBar->setMinimumHeight(d->window->titlebar()->height());
    }    

    inputWidget->setParent(d->window);
    d->centrals.insert(navName, inputWidget);
}

void WindowKeeper::addMenu(AbstractMenu *menu)
{
    qInfo() << __FUNCTION__;

    DMenu *inputMenu = static_cast<DMenu*>(menu->qMenu());
    if (!d->window || !inputMenu)
        return;

    //始终将Helper置末
    for (QAction *action : d->mainMenu->actions()) {
        if (action->text() == MWM_TOOLS) {
            d->mainMenu->insertMenu(action, inputMenu);
            return; //提前返回
        }
    }

    //直接添加到最后
    d->mainMenu->addMenu(inputMenu);
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
    QAction *inputAction = static_cast<QAction*>(action->qAction());
    if (!action || !inputAction)
        return;

    if (menuName == MWMFA_NEW_FILE_OR_PROJECT) {
        for (QAction *qAction : d->mainMenu->actions()) {
            if (qAction->text() == MWM_BUILD) {                
                d->mainMenu->insertAction(qAction, inputAction);
                d->mainMenu->insertSeparator(qAction);
                return;
            }
        }
    }

    if (menuName == MWM_ANALYZE) {
        for (QAction *qAction : d->mainMenu->actions()) {
            if (qAction->text() == MWM_TOOLS) {
                for (QAction *toolAction : qAction->menu()->actions()) {
                    if (toolAction->text() == MWMTA_BINARY_TOOLS) {
                        qAction->menu()->insertAction(toolAction, inputAction);
                        return;
                    }
                }
            }
        }
    }

    if (menuName == MWM_TOOLS && inputAction->text() == MWMTA_OPTIONS) {
        for (QAction *qAction : d->mainMenu->actions()) {
            if (qAction->text() == "Help") {
                d->mainMenu->insertAction(qAction, inputAction);
            }
            if (qAction->text() == "About") {
                d->mainMenu->removeAction(qAction);
                return;
            }
        }
    }

    for (QAction *qAction : d->mainMenu->actions()) {
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
    if (!action || !action->qAction())
        return;

    QAction *inputAction = static_cast<QAction*>(action->qAction());

    foreach (QAction *action, d->mainMenu->actions()) {
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

void WindowKeeper::initUserWidget()
{
    qApp->processEvents();
    if (!d->leftToolBar)
        return;

//    if (d->toolbar->actions().size() > 0) {
//        d->toolbar->actions().at(0)->trigger();
//    }
}

void WindowKeeper::switchWidgetNavigation(const QString &navName)
{
    auto beforWidget = d->window->takeCentralWidget();
    if (beforWidget) {
        beforWidget->hide();
    }

    if (d->centrals.isEmpty() || !d->window)
        return;

    auto widget = d->centrals[navName];
    if (!widget)
        return;

    for (auto btn : d->topToolBtnList) {
        btn->hide();
    }

    d->window->titlebar()->setTitle(QString(tr("Deepin Union Code")));

    if (navName == MWNA_EDIT) {
        for (auto action : d->editToolBar->actions()) {
            DIconButton *iconBtn = new DIconButton();
            iconBtn->setIcon(action->icon());
            iconBtn->setMinimumSize(QSize(36, 36));
            iconBtn->setIconSize(QSize(20, 20));
            d->topToolBtnList.append(iconBtn);

            connect(iconBtn, &DIconButton::clicked, action, &QAction::triggered);

            QHBoxLayout *topToolBarLayout = static_cast<QHBoxLayout*>(d->topToolBar->layout());
            topToolBarLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            topToolBarLayout->addWidget(iconBtn);
        }
        QHBoxLayout *titleBarLayout = static_cast<QHBoxLayout*>(d->window->titlebar()->layout());
        titleBarLayout->insertWidget(1, d->topToolBar, Qt::AlignLeft);
        titleBarLayout->insertSpacing(titleBarLayout->indexOf(d->topToolBar), 20);
        d->window->titlebar()->setTitle(QString());
        d->topToolBar->show();
    }
    setNavActionChecked(navName, true);

    showNavWidget(widget);
}

void WindowKeeper::showNavWidget(DWidget *widget)
{
    DWidget *navWidget = new DWidget();
    QHBoxLayout *layout = new QHBoxLayout(navWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(d->leftToolBar);
    layout->addWidget(widget);

    d->window->setCentralWidget(navWidget);
    d->window->centralWidget()->show();
}

void WindowKeeper::setNavActionChecked(const QString &actionName, bool checked)
{
    if (!d->navActionGroup)
        return;

    for (auto it = d->leftToolBtns.begin(); it != d->leftToolBtns.end(); it++) {
        it.value()->setChecked(false);
        if (it.key() == actionName) {
            it.value()->setChecked(checked);
        }
    }
}

void WindowKeeper::showAboutDlg()
{
    AboutDialog dlg;
    dlg.exec();
}

void WindowKeeper::showAboutPlugins()
{
    PluginDialog dialog;
    dialog.exec();
}
