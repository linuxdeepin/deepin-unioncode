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
#include "eventsender.h"
#include "framework.h"
#include "common/common.h"

void EventSender::jumpTo(dap::string filepath, int numberline)
{
    if (!filepath.c_str())
        return;

    dpf::Event event;
    event.setTopic(T_DEBUGGER);
    event.setData(D_DEBUG_EXECUTION_JUMP);
    event.setProperty(P_FILEPATH, QString(filepath.c_str()));
    event.setProperty(P_FILELINE, numberline);
    dpf::EventCallProxy::instance().pubEvent(event);
}
