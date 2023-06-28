// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAVEDITMAINWINDOW_H
#define NAVEDITMAINWINDOW_H

#include <QMainWindow>
#include <QMutex>

class QGridLayout;
class QTabWidget;
class AbstractCentral;
class AbstractWidget;
class AbstractConsole;
class QDockWidget;
class AutoHideDockWidget;
class ToolBarManager;

class NavEditMainWindow : public QMainWindow
{
    Q_OBJECT
    AutoHideDockWidget *qDockWidgetWorkspace{nullptr};
    AutoHideDockWidget *qDockWidgetContext{nullptr};
    AutoHideDockWidget *qDockWidgetWatch{nullptr};
    AutoHideDockWidget *qDockWidgetFindToolBar{nullptr};
    AutoHideDockWidget *qDockWidgetValgrindBar{nullptr};
    AutoHideDockWidget *qDockWidgetTools{nullptr};
    QWidget *qWidgetEdit{nullptr};
    QWidget *qWidgetWatch{nullptr};
    QTabWidget *qTabWidgetContext{nullptr};
    QTabWidget *qTabWidgetWorkspace{nullptr};
    QTabWidget *qTabWidgetTools{nullptr};
    ToolBarManager *mainToolBar{nullptr};
    QList<QString> contextList;
    QMutex mutex;
public:
    static NavEditMainWindow *instance();
    explicit NavEditMainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~NavEditMainWindow();
    QStringList contextWidgetTitles() const;
    void addWidgetWorkspace(const QString &title, AbstractWidget *treeWidget);
    // return old edit widget
    QWidget *setWidgetEdit(AbstractCentral *editWidget);
    // return old watch widget
    QWidget *setWidgetWatch(AbstractWidget *watchWidget);
    void addContextWidget(const QString &title, AbstractWidget *contextWidget, const QString &group);
    // the page widget itself is not deleted.
    void removeContextWidget(AbstractWidget *contextWidget);
    bool switchWidgetWorkspace(const QString &title);
    bool switchWidgetContext(const QString &title);
    bool switchWidgetTools(const QString &title);
    void addFindToolBar(AbstractWidget *findToolbar);
    void showFindToolBar();
    void addValgrindBar(AbstractWidget *valgrindbar);
    void showValgrindBar();
    bool addToolBarActionItem(const QString &id, QAction *action, const QString &group);
    bool addToolBarWidgetItem(const QString &id, AbstractWidget *widget, const QString &group);
    void removeToolBarItem(const QString &id);
    void setToolBarItemDisable(const QString &id, bool disable);

private:
    void adjustWorkspaceItemOrder();
};

#endif // NAVEDITMAINWINDOW_H
