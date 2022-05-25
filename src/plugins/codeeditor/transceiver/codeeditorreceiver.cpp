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
#include "codeeditorreceiver.h"
#include "common/common.h"
#include "services/project/projectservice.h"

static QStringList subTopics
{
    T_MENU, T_FILEBROWSER , T_DEBUGGER, T_PROJECT
};

CodeEditorReceiver::CodeEditorReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<CodeEditorReceiver> ()
{

}

dpf::EventHandler::Type CodeEditorReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList CodeEditorReceiver::topics()
{
    return subTopics; //绑定menu 事件
}

void CodeEditorReceiver::eventProcess(const dpf::Event &event)
{
    if (!subTopics.contains(event.topic()))
        abort();

    if (T_DEBUGGER == event.topic()) {
        eventDebugger(event);
    } else if (T_PROJECT == event.topic()) {
        eventProject(event);
    } else if (T_MENU == event.topic()) {
        eventMenu(event);
    } else if (T_FILEBROWSER == event.topic()) {
        eventFileBrowser(event);
    }
}

void CodeEditorReceiver::eventFileBrowser(const dpf::Event &event)
{
    if (D_ITEM_DOUBLECLICKED == event.data()) {
        auto proInfo = qvariant_cast<dpfservice::ProjectInfo>(event.property(P_PROJECT_INFO));
        return DpfEventMiddleware::instance()->toOpenFile(
                    Head { proInfo.workspaceFolder(), proInfo.language()} ,
                    event.property(P_FILEPATH).toString());
    }
}

void CodeEditorReceiver::eventDebugger(const dpf::Event &event)
{
    if (D_DEBUG_EXECUTION_JUMP == event.data()) {
        return DpfEventMiddleware::instance()->toRunFileLine(
                    event.property(P_FILEPATH).toString(),
                    event.property(P_FILELINE).toInt());
    }

    if (D_DEBUG_EXECUTION_JUMP_CLEAN == event.data()) {
        return DpfEventMiddleware::instance()->toRunClean();
    }

    if (D_MARGIN_DEBUG_POINT_REMOVE == event.data()) {
        return DpfEventMiddleware::instance()->toDebugPointClean();
    }
}

void CodeEditorReceiver::eventProject(const dpf::Event &event)
{
    if (D_ITEM_DOUBLECLICKED == event.data()) {
        Head head{ event.property(P_WORKSPACEFOLDER).toString(),
                    event.property(P_LANGUAGE).toString() };
        return DpfEventMiddleware::instance()->toOpenFile(
                    head,
                    event.property(P_FILEPATH).toString());
    }
}

void CodeEditorReceiver::eventMenu(const dpf::Event &event)
{
    if (D_FILE_OPENDOCUMENT == event.data()) {
        return DpfEventMiddleware::instance()->toOpenFile(
                    event.property(P_FILEPATH).toString());
    }
    if (D_FILE_OPENFOLDER == event.data()) {
        qInfo() << event;
        return;
    }
}

DpfEventMiddleware *DpfEventMiddleware::instance()
{
    static DpfEventMiddleware ins;
    return &ins;
}
