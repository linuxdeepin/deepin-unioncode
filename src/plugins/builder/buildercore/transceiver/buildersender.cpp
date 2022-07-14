/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "buildersender.h"
#include "framework.h"
#include "common/common.h"

BuilderSender::BuilderSender(QObject *parent)
    : QObject(parent)
{

}

BuilderSender::~BuilderSender()
{

}

void BuilderSender::jumpTo(const QString &filePath, int lineNum)
{
    if (filePath.isEmpty() || lineNum < 0)
        return;

    dpf::Event event;
    event.setTopic(T_DEBUGGER);
    event.setData(D_DEBUG_EXECUTION_JUMP);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, lineNum);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void BuilderSender::notifyBuildState(BuildState state, const BuildCommandInfo &commandInfo)
{
    dpf::Event event;
    event.setTopic(T_BUILDER);
    event.setData(D_BUILD_STATE);
    event.setProperty(P_STATE, static_cast<int>(state));
    event.setProperty(P_ORIGINCMD, QVariant::fromValue(commandInfo));
    dpf::EventCallProxy::instance().pubEvent(event);
}

