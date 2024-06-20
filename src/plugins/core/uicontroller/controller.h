// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "mainwindow.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"

#include <DToolButton>

#include <QObject>
#include <QMap>
#include <QStringList>

class ControllerPrivate;
class AbstractModule;
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller();
    static Controller *instance();

    void registerModule(const QString &moduleName, AbstractModule *module);

public slots:
    //mode : MD_EDIT/MD_DEBUG/MD_RECENT
    void raiseMode(const QString &mode);

    //1 Plugi -> 1navName -> n widget(name)
    void setCurrentPlugin(const QString &plugin);
    void registerWidgetToMode(const QString &name, AbstractWidget *abstractWidget, const QString &mode, Position pos, bool replace, bool isVisible);
    void registerWidget(const QString &name, AbstractWidget *abstractWidget);
    void replaceWidget(const QString &name, Position pos = Position::FullWindow);
    void insertWidget(const QString &name, Position pos, Qt::Orientation orientation);
    void hideWidget(const QString &name);
    void showWidgetAtPosition(const QString &name, Position pos, bool replace = true);

    void addNavigationItem(AbstractAction *action, quint8 priority);
    void addNavigationItemToBottom(AbstractAction *action, quint8 priority);
    void switchWidgetNavigation(const QString &navName);

    //bottom contextWidget
    void addContextWidget(const QString &title, AbstractWidget *contextWidget, bool isVisible);
    bool hasContextWidget(const QString &title);
    void showContextWidget();
    void hideContextWidget();
    void switchContextWidget(const QString &title);

    //menu
    void addChildMenu(AbstractMenu *menu);
    void insertAction(const QString &menuName, const QString &beforActionName,
                      AbstractAction *action);
    void addAction(const QString &menuName, AbstractAction *action);
    void removeActions(const QString &menuName);
    void addOpenProjectAction(const QString &name, AbstractAction *action);

    //topToolBar
    void addWidgetToTopTool(AbstractWidget *abstractWidget, const QString &group, bool addSeparator, bool addToLeft);
    void addTopToolItem(AbstractAction *action, const QString &group, bool addSeparator);
    void addTopToolItemToRight(AbstractAction *action, bool addSeparator);
    void showTopToolBar(const QString &group);

    void openFileDialog();

    //statusBar
    void showStatusBar();
    void hideStatusBar();

    //workspace
    void switchWorkspace(const QString &titleName);

private:
    ControllerPrivate *d;

    void registerService();

    //show loadingwidget
    void loading();

    void initMainWindow();
    void initNavigationBar();
    void initMenu();
    void initContextWidget();
    void initStatusBar();
    void initWorkspaceWidget();
    void initTopToolBar();
    void initModules();

    //menu
    void createHelpActions();
    void createToolsActions();
    void createDebugActions();
    void createBuildActions();
    void createFileActions();

    void addMenuShortCut(QAction *action, QKeySequence keySequence = QKeySequence());

    void registerActionShortCut(AbstractAction *action);

    void showWorkspace();

    DToolButton *createIconButton(QAction *action);
};

#endif   // CONTROLLER_H
