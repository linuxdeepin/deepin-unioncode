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

#include <framework/framework.h>

#include <QObject>

class QAction;
class QMainWindow;
class QStatusBar;
class QMenuBar;
class QToolBar;
class QActionGroup;

extern QHash<QString, QWidget *> centrals;
extern QMainWindow *window;
extern QActionGroup *navActionGroup;
extern QToolBar *toolbar;
class WindowKeeper final :public QObject
{
    Q_OBJECT
    friend class WindowSwitcher;

public:
    explicit WindowKeeper(QObject *parent = nullptr);

public slots:
    void addNavAction(AbstractAction *action);
    void addCentral(const QString &navName, AbstractCentral *central);
    void addMenu(AbstractMenu *menu);
    void addAction(const QString &menuName, AbstractAction *action);
    void initUserWidget();

private :
    void layoutWindow(QMainWindow *window);
    void createNavRuntime(QToolBar *toolbar);
    void createNavDebug(QToolBar *toolbar);
    void createNavEdit(QToolBar *toolbar);
    void createNavRecent(QToolBar *toolbar);
    void createStatusBar(QMainWindow *window);
    void createHelpActions(QMenuBar *menuBar);
    void createToolsActions(QMenuBar *menuBar);
    void createDebugActions(QMenuBar *menuBar);
    void createBuildActions(QMenuBar *menuBar);
    void createFileActions(QMenuBar *menuBar);
};

#endif // WINDOWKEEPER_H
