// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unistd.h"
#include "debugengine.h"

#include "dapsession.h"

#include "debugger/java/javadebugger.h"
#include "debugger/python/pythondebugger.h"

#include <QDBusConnection>

DebugEngine::DebugEngine(QObject *parent)
    : QObject(parent)
{
    dapSession.reset(new DapSession());
    javaDebugger.reset(new JavaDebugger());
    pythonDebugger.reset(new PythonDebugger());

    listeningIdeAlive();
}

bool DebugEngine::start()
{
    return dapSession->start();
}

void DebugEngine::stop()
{
    dapSession->stop();
}

bool DebugEngine::exit()
{
    // TODO(mozart)
    return true;
}

bool DebugEngine::initialize()
{
    // TODO(mozart)
    return true;
}

void DebugEngine::listeningIdeAlive()
{
    checkIdeAliveTimer.setInterval(1000);
    connect(&checkIdeAliveTimer, &QTimer::timeout, this, [=](){
        if(getppid() == 1)
            ::exit(0);
    });
    checkIdeAliveTimer.start();
}
