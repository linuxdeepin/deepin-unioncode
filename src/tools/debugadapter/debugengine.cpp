// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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


