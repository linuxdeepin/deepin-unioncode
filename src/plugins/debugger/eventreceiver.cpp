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
#include "eventreceiver.h"
#include "common/util/eventdefinitions.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"


static QStringList subTopics{T_CODEEDITOR};
EventReceiver::EventReceiver(QObject *parent)
    : dpf::EventHandler(parent)
    , dpf::AutoEventHandlerRegister<EventReceiver>()
{

}

dpf::EventHandler::Type EventReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList &EventReceiver::topics()
{
    return subTopics;
}

void EventReceiver::eventProcess(const dpf::Event &event)
{
    qInfo() << event;
    if (!topics().contains(event.topic()))
        return;

    QString topic = event.topic();
    QString data = event.data().toString();
    if (topic == T_CODEEDITOR) {
        if (data == D_MARGIN_DEBUG_POINT_ADD) {
            QString filePath = event.property(P_FILEPATH).toString();
            int lineNumber = event.property(P_FILELINE).toInt();
            emit debuggerSignals->breakpointAdded(filePath, lineNumber);
        }
    }
}
