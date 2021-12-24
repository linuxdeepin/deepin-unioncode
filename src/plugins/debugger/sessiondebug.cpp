/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "sessiondebug.h"

#include "io.h"
#include "protocol.h"
#include "session.h"

using namespace dap;
SessionDebug::SessionDebug(QObject *parent) : QObject(parent)
{

}

void SessionDebug::setDebugger(AbstractDebugger *dbg)
{
    debugger = dbg;
}

bool SessionDebug::initialize()
{
    bool bRet = false;
    auto tmpSession = dap::Session::create();
    if (tmpSession.get()) {
        session = std::move(tmpSession);
        bRet = true;
    }

    bRet &= registerHandlers();

    return bRet;
}

bool SessionDebug::registerHandlers()
{
    return true;
}

void SessionDebug::onError(const char *err)
{
    Q_UNUSED(err)
    //TODO(mozart):display the error.
}
