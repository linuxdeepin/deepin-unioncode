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
#include "workspacedata.h"
#include "common/common.h"

static QStringList subTopics { T_MENU, T_NAV };
static QString projectPath;
WorkspaceReceiver::WorkspaceReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<WorkspaceReceiver> ()
{

}

dpf::EventHandler::Type WorkspaceReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
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
            projectPath = event.property(P_FILEPATH).toString();
        }
    }
    if (event.topic() == T_NAV) {
        if (event.data() == D_ACTION_SHOW) {
            if (event.property(P_ACTION_TEXT).toString() == MWNA_EDIT) {
                WorkspaceData::globalInstance()->doGenerate(projectPath);
            }
        }
    }
}
