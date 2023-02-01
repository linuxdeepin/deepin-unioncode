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
#ifndef NAVEDITMAINWINDOW_H
#define NAVEDITMAINWINDOW_H

#include <QMainWindow>

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
    QWidget *qWidgetEdit{nullptr};
    QWidget *qWidgetWatch{nullptr};
    QTabWidget *qTabWidgetContext{nullptr};
    QTabWidget *qTabWidgetWorkspace{nullptr};
    ToolBarManager *mainToolBar{nullptr};
public:
    static NavEditMainWindow *instance();
    explicit NavEditMainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~NavEditMainWindow();
    QStringList contextWidgetTitles() const;
    void setConsole(AbstractConsole *console);
    void addWidgetWorkspace(const QString &title, AbstractWidget *treeWidget);
    void setWidgetEdit(AbstractCentral *editWidget);
    void setWidgetWatch(AbstractWidget *watchWidget);
    void addWidgetContext(const QString &title, AbstractWidget *contextWidget);
    bool switchWidgetWorkspace(const QString &title);
    bool switchWidgetContext(const QString &title);
    void addFindToolBar(AbstractWidget *findToolbar);
    void showFindToolBar();
    bool addToolBarActionItem(const QString &id, QAction *action);
    bool addToolBarWidgetItem(const QString &id, AbstractWidget *widget);
    void addToolBarSeparator();
    void removeToolBarItem(const QString &id);
    void setToolBarItemDisable(const QString &id, bool disable);
};

#endif // NAVEDITMAINWINDOW_H
