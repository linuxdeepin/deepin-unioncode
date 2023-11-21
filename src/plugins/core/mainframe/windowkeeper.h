// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWKEEPER_H
#define WINDOWKEEPER_H

#include "base/abstractnav.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"
#include "base/abstractmenubar.h"
#include "base/abstractmainwindow.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"

#include <framework/framework.h>

#include <DApplication>
#include <DMainWindow>
#include <DToolBar>
#include <DMenuBar>

#include <QObject>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class QAction;
class QMainWindow;
class QStatusBar;
class QMenuBar;
class QToolBar;
class QActionGroup;

class WindowKeeperPrivate;
class WindowKeeper final :public QObject
{
    Q_OBJECT
    WindowKeeperPrivate *const d;
    friend class WindowSwitcher;

public:
    static WindowKeeper *instace();
    explicit WindowKeeper(QObject *parent = nullptr);
    virtual ~WindowKeeper();
    QStringList navActionTexts() const;

public slots:
    void addNavigation(const QString &navName, const QString &iconName);
    void addCentralNavigation(const QString &navName, AbstractCentral *central);
    void addMenu(AbstractMenu *menu);
    void insertAction(const QString &menuName, const QString &beforActionName,
                      AbstractAction *action);
    void addAction(const QString &menuName, AbstractAction *action);
    void removeActions(const QString &menuName);
    void addOpenProjectAction(const QString &name, AbstractAction *action);
    void initUserWidget();
    void switchWidgetNavigation(const QString &navName);

    void setTopToolBarWidget(AbstractWidget *widget);

private :
    void showAboutDlg();
    void showAboutPlugins();
    void layoutWindow(DMainWindow *window);
    void createStatusBar(DMainWindow *window);

    void createHelpActions(DMenu *menu);
    void createToolsActions(DMenu *menu);
    void createDebugActions(DMenu *menu);
    void createBuildActions(DMenu *menu);
    void createFileActions(DMenu *menu);

    void setNavActionChecked(const QString &actionName, bool checked);

    void createMainMenu(DMenu *mainMenu);
    void initLeftToolBar();
    void createNavIconBtn(const QString &navName, const QString &iconName);
};

#endif // WINDOWKEEPER_H
