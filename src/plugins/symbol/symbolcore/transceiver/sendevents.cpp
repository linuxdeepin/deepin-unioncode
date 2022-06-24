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
#include "framework/framework.h"
#include "common/common.h"

void SendEvents::doubleCliekedJumpToLine(const QString &workspace,
                                         const QString &language, const QString &filePath, int line)
{
    dpf::Event event;
    event.setTopic(T_SYMBOL);
    event.setData(D_JUMP_TO_LINE);
    event.setProperty(P_WORKSPACEFOLDER, workspace);
    event.setProperty(P_LANGUAGE, language);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, line);
    dpf::EventCallProxy::instance().pubEvent(event);
}
