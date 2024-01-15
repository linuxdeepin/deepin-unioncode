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

struct View
{
    View() {}
    QString pluginName;
    QString previousView;

    QStringList widgetList;
    QStringList topToolItemList;
    bool showContextWidget { false };
    QList<Position> hiddenposList;
};

class ControllerPrivate;
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = nullptr);
    ~Controller();
    static Controller *instance();

signals:
    void switchWidget(const QString &PluginName);

public slots:
    void raiseView(const QString &plugin);
    bool hasView(const QString &plugin);
    void setCurrentPlugin(const QString &plugin);
    //1 Pluginname == 1navName -> n name - n widget
    void setPreviousView(const QString &pluginName, const QString &previous);
    void addWidget(const QString &name, AbstractWidget *abstractWidget, Position pos = Position::FullWindow, bool replace = true);

    void addNavigation(AbstractAction *action);
    void addNavigationToBottom(AbstractAction *action);
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
    void addTopToolItem(const QString &name, AbstractAction *action);

    void openFileDialog();
    void showAboutPlugins();

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

    //menu
    void createHelpActions();
    void createToolsActions();
    void createDebugActions();
    void createBuildActions();
    void createFileActions();

    void addMenuShortCut(QAction *action, QKeySequence keySequence = QKeySequence());
};

#endif   // CONTROLLER_H
