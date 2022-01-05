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
#include "debugger.h"
#include "dap/io.h"
#include "dap/protocol.h"
#include "debugsession.h"
#include "stdlib.h"

/**
 * @brief Debugger::Debugger
 * For serial debugging service
 */
Debugger::Debugger(QObject *parent) : QObject (parent)
{
    session.reset(new DebugSession(this));
}

void Debugger::startDebug()
{
    session->initialize();
}

void Debugger::detachDebug()
{
    session->attach();
}

void Debugger::interruptDebug()
{
    // Just use temporary parameters now, same for the back
    int threadId = 1;
    session->pause(threadId);
}

void Debugger::continueDebug()
{
    int threadId = 1;
    session->continueDbg(threadId);
}

void Debugger::abortDebug()
{
    session->terminate();
}

void Debugger::restartDebug()
{
    session->restart();
}

void Debugger::stepOver()
{
    int threadId = 1;
    const char *granularity = "statement";
    session->next(threadId, granularity);
}

void Debugger::stepIn()
{
    int threadId = 1;
    int targetId = 1;
    const char *granularity = "statement";
    session->stepIn(threadId, targetId, granularity);
}

void Debugger::stepOut()
{
    int threadId = 1;
    const char *granularity = "statement";
    session->stepOut(threadId, granularity);
}
