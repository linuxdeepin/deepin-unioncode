/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include "debuggerglobals.h"
#include "interface/stackframemodel.h"
#include "interface/localtreemodel.h"
#include "interface/variable.h"
#include "interface/breakpointmodel.h"
#include "dap/debugger.h"

#include <QTreeView>
#include <QSharedPointer>
#include <QObject>

class AppOutputPane;
class StackFrameView;
class MenuManager;
namespace dpfservice {
    class WindowService;
}
class DebugManager : public QObject
{
    Q_OBJECT
public:
    explicit DebugManager(QObject *parent = nullptr);
    bool initialize(dpfservice::WindowService *service);
    AppOutputPane *getOutputPane() const;
    QTreeView *getStackPane() const;
    QTreeView *getLocalsPane() const;
    QTreeView *getBreakpointPane() const;

signals:
    void debugStarted();

public slots:
    /**
     * UI triggered.
     */
    void run();
    void detachDebug();

    void interruptDebug();
    void continueDebug();
    void abortDebug();
    void restartDebug();

    void stepOver();
    void stepIn();
    void stepOut();

    void handleRunStateChanged(Debugger::RunState state);

private:
    void launchBackend();

    QSharedPointer<Debugger> debugger;
    QProcess backend;

    QSharedPointer<MenuManager> menuManager;
};

#endif   // DEBUGMANAGER_H
