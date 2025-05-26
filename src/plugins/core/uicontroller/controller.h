// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "mainwindow.h"

#include "base/abstractmenu.h"
#include "base/abstractwidget.h"
#include "common/actionmanager/command.h"

#include <DToolButton>

#include <QObject>
#include <QMap>
#include <QStringList>

class ControllerPrivate;
class AbstractModule;
class WidgetInfo;
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller();
    static Controller *instance();

    void registerModule(const QString &moduleName, AbstractModule *module);
    MainWindow *mainWindow() const;

public slots:
    //mode : MD_EDIT/MD_DEBUG/MD_RECENT
    void raiseMode(const QString &mode);

    //1 Plugi -> 1navName -> n widget(name)
    void setCurrentPlugin(const QString &plugin);
    void registerWidgetToMode(const QString &name, AbstractWidget *abstractWidget, const QString &mode, Position pos, bool replace, bool isVisible);
    void registerWidget(const QString &name, AbstractWidget *abstractWidget);
    void registerWidgetCreator(const QString &name, std::function<AbstractWidget*()> &widgetCreateFunc);
    void replaceWidget(const QString &name, Position pos = Position::FullWindow);
    void insertWidget(const QString &name, Position pos, Qt::Orientation orientation);
    void hideWidget(const QString &name);
    void showWidgetAtPosition(const QString &name, Position pos, bool replace = true);
    void resizeDocks(const QList<QString> &docks, const QList<int> &sizes, Qt::Orientation orientation);
    void setDockHeaderName(const QString &dockName, const QString &headerName);
    void setDockHeaderList(const QString &dockName, const QList<QAction *> &actions);
    void deleteDockHeader(const QString &dockName);

    void addNavigationItem(AbstractAction *action, quint8 priority);
    void addNavigationItemToBottom(AbstractAction *action, quint8 priority);
    void switchWidgetNavigation(const QString &navName);
    void bindWidgetToNavigation(const QString &dockName, AbstractAction *action);

    void registerWidgetToDockHeader(const QString &dockName, QWidget *widget);

    //bottom contextWidget
    void addContextWidget(const QString &title, AbstractWidget *contextWidget, bool isVisible);
    bool hasContextWidget(const QString &title);
    void showContextWidget();
    void hideContextWidget();
    void switchContextWidget(const QString &title);

    //topToolBar
    void addWidgetToTopTool(AbstractWidget *abstractWidget, bool addSeparator, bool addToLeft, quint8 priority);
    DToolButton *addTopToolItem(Command *action, bool addSeparator, quint8 priority);
    DToolButton *addTopToolItemToRight(Command *action, bool addSeparator, quint8 priority);
    void showTopToolBar();
    void removeTopToolItem(Command *action);
    void setTopToolItemVisible(Command *action, bool visible);

    void openFileDialog();

    //statusBar
    void showStatusBar();
    void hideStatusBar();
    void addStatusBarItem(QWidget *item);

    //workspace
    void switchWorkspace(const QString &titleName);

    void showRightspace();
    void showCurrentDocksManager();

private:
    ControllerPrivate *d;

    void registerService();
    void createDockWidget(WidgetInfo &info);
    bool checkDocksManager();
    DToolButton *createDockButton(const WidgetInfo &info);  // dock`s status switcher

    //show loadingwidget
    void loading();

    void initMainWindow();
    void initNavigationBar();
    void initContextWidget();
    void initStatusBar();
    void initWorkspaceWidget();
    void initRightspaceWidget();
    void initTopToolBar();
    void initModules();
    void initDocksManager();

    //menu and action
    void registerDefaultContainers();
    void registerDefaultActions();

    void showWorkspace();

    DToolButton *createIconButton(QAction *action);  // top tool
};

#endif   // CONTROLLER_H
