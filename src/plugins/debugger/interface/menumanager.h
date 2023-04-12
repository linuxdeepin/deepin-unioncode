/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <dap/dapdebugger.h>

#include <QObject>
#include <QAction>

namespace dpfservice {
class WindowService;
}

class MenuManager : public QObject
{
    Q_OBJECT
public:
    explicit MenuManager(QObject *parent = nullptr);

    void initialize(dpfservice::WindowService *service);

    void handleRunStateChanged(AbstractDebugger::RunState state);

signals:

public slots:


private:
    QSharedPointer<QAction> startDebugging;
    QSharedPointer<QAction> detachDebugger;
    QSharedPointer<QAction> interrupt;
    QSharedPointer<QAction> continueDebugging;
    QSharedPointer<QAction> abortDebugging;
    QSharedPointer<QAction> restartDebugging;
    QSharedPointer<QAction> stepOver;
    QSharedPointer<QAction> stepIn;
    QSharedPointer<QAction> stepOut;
};

#endif // MENUMANAGER_H
