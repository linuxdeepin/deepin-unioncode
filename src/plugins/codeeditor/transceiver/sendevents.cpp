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
#include "sendevents.h"
#include "framework.h"
#include "common/common.h"

void SendEvents::marginDebugPointAdd(const QString filePath, quint64 fileLine)
{
    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_MARGIN_DEBUG_POINT_ADD);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, fileLine);
    dpf::EventCallProxy::instance().pubEvent(event);
    qInfo() << event;
}

void SendEvents::marginDebugPointRemove(const QString filePath, quint64 fileLine)
{
    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_MARGIN_DEBUG_POINT_REMOVE);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, fileLine);
    dpf::EventCallProxy::instance().pubEvent(event);
    qInfo() << event;
}

void SendEvents::sendCurrentEditFileStatus(const QString &filePath, bool actived)
{
    dpf::Event event;
    event.setTopic(T_PROJECT);
    event.setData(D_FILE_OPENDOCUMENT);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_OPRATETYPE, actived);
    dpf::EventCallProxy::instance().pubEvent(event);
}
