// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    QSharedPointer<QAction> attachDebugging;
    QSharedPointer<QAction> detachDebugger;
    QSharedPointer<QAction> interrupt;
    QSharedPointer<QAction> continueDebugging;
    QSharedPointer<QAction> reverseContinue;
    QSharedPointer<QAction> abortDebugging;
    QSharedPointer<QAction> restartDebugging;
    QSharedPointer<QAction> stepOver;
    QSharedPointer<QAction> stepIn;
    QSharedPointer<QAction> stepOut;
    QSharedPointer<QAction> stepBack;
    QSharedPointer<QAction> remoteDebug;
};

#endif // MENUMANAGER_H
