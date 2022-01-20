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
#include "workspacereceiver.h"
#include "common/util/supportfile.h"
#include "common/util/eventdefinitions.h"

static QStringList subTopics { T_MENU };

WorkspaceReceiver::WorkspaceReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<WorkspaceReceiver> ()
{

}

dpf::EventHandler::Type WorkspaceReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList WorkspaceReceiver::topics()
{
    return subTopics; //绑定menu 事件
}

void WorkspaceReceiver::eventProcess(const dpf::Event &event)
{
    if (!subTopics.contains(event.topic()))
        abort();
    qInfo() << event;
    if (event.topic() == T_MENU) {
        if (event.data() == D_FILE_OPENFOLDER) {
            auto val = SupportFile::Builder::buildInfos(event.property(P_FILEPATH).toString());
            int a = 20;
            return;
        }
    }
}
