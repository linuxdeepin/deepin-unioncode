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


