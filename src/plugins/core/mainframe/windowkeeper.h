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

#include <QObject>

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
    void addActionNavigation(const QString &id, AbstractAction *action);
    void addCentralNavigation(const QString &navName, AbstractCentral *central);
    void addMenu(AbstractMenu *menu);
    void insertAction(const QString &menuName, const QString &beforActionName,
                      AbstractAction *action);
    void addAction(const QString &menuName, AbstractAction *action);
    void removeActions(const QString &menuName);
    void addOpenProjectAction(const QString &name, AbstractAction *action);
    void initUserWidget();
    void switchWidgetNavigation(const QString &navName);

private :
    void showAboutDlg();
    void showAboutPlugins();
    void layoutWindow(QMainWindow *window);
    void createNavEdit(QToolBar *toolbar);
    void createNavRecent(QToolBar *toolbar);
    void createStatusBar(QMainWindow *window);
    void createHelpActions(QMenuBar *menuBar);
    void createToolsActions(QMenuBar *menuBar);
    void createDebugActions(QMenuBar *menuBar);
    void createBuildActions(QMenuBar *menuBar);
    void createFileActions(QMenuBar *menuBar);
    void createAnalyzeActions(QMenuBar *menuBar);
    void setNavActionChecked(const QString &actionName, bool checked);
};

#endif // WINDOWKEEPER_H
