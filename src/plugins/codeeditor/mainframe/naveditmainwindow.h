// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAVEDITMAINWINDOW_H
#define NAVEDITMAINWINDOW_H

#include <DMainWindow>
#include <DWidget>
#include <DFrame>
#include <DTabWidget>
#include <DToolButton>

#include <DStackedWidget>

#include <QMutex>

DWIDGET_USE_NAMESPACE

class QGridLayout;
class QTabWidget;
class AbstractCentral;
class AbstractWidget;
class AbstractConsole;
class QDockWidget;
class AutoHideDockWidget;
class ToolBarManager;

class NavEditMainWindow : public DMainWindow
{
    Q_OBJECT

public:
    static NavEditMainWindow *instance();
    explicit NavEditMainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~NavEditMainWindow();

    void addWidgetWorkspace(const QString &title, AbstractWidget *treeWidget, const QString &iconName);
    void addWorkspaceArea(const QString &title, AbstractWidget *widget);
    void switchWorkspaceArea(const QString &title);

    // return old edit widget
    DWidget *setWidgetEdit(AbstractCentral *editWidget);
    // return old watch widget
    DWidget *setWidgetWatch(AbstractWidget *watchWidget);
    void addContextWidget(const QString &title, AbstractWidget *contextWidget, const QString &group, bool isVisibal);
    bool hasContextWidget(const QString &title);
    // the page widget itself is not deleted.
    void removeContextWidget(AbstractWidget *contextWidget);
    bool switchWidgetWorkspace(const QString &title);
    bool switchWidgetContext(const QString &title);
    bool switchWidgetTools(const QString &title);
    void addFindToolBar(AbstractWidget *findToolbar);
    void showFindToolBar();
    void addValgrindBar(AbstractWidget *valgrindbar);
    void showValgrindBar();
    bool addToolBarWidgetItem(const QString &id, AbstractWidget *widget, const QString &group);
    void removeToolBarItem(const QString &id);
    void setToolBarItemDisable(const QString &id, bool disable);

private:
    void adjustWorkspaceItemOrder();
    void initWorkspaceUI();
    void initContextUI();

    AutoHideDockWidget *qDockWidgetWorkspace{nullptr};
    AutoHideDockWidget *qDockWidgetContext{nullptr};
    AutoHideDockWidget *qDockWidgetWatch{nullptr};
    AutoHideDockWidget *qDockWidgetFindToolBar{nullptr};
    AutoHideDockWidget *qDockWidgetValgrindBar{nullptr};
    AutoHideDockWidget *qDockWidgetTools{nullptr};
    DWidget *qWidgetEdit{nullptr};
    DWidget *qWidgetWatch{nullptr};

    DTabWidget *qTabWidgetWorkspace{nullptr};
    DTabWidget *qTabWidgetTools{nullptr};
    ToolBarManager *mainToolBar{nullptr};
    QMap<QString, DWidget*> workspaceWidgets;
    QMutex mutex;

    QMap<QString, DWidget*> contextWidgets;
    QMap<QString, DPushButton*> tabButtons;
    DFrame *contextTabBar {nullptr};
    DStackedWidget *stackContextWidget {nullptr};
    DWidget *contextWidget {nullptr};

    DWidget *editWorkspaceWidget {nullptr};
    QMap<QString, DWidget*> editWorkspaceWidgets;
    QMap<QString, DToolButton*> workspaceTabButtons;
    DStackedWidget *stackEditWorkspaceWidget {nullptr};
    DFrame *workspaceTabBar {nullptr};
};

#endif // NAVEDITMAINWINDOW_H
