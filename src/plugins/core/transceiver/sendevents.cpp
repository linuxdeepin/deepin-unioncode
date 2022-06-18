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
#include "common/common.h"
#include "services/window/windowservice.h"
#include <framework/framework.h>

void SendEvents::navActionSwitch(const QString &navName)
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavRecent;
    showNavRecent.setTopic(T_NAV);
    showNavRecent.setData(D_ACTION_SWITCH);
    showNavRecent.setProperty(P_ACTION_TEXT, navName);
    dpf::EventCallProxy::instance().pubEvent(showNavRecent);
}

void SendEvents::menuOpenFile(const QString &filePath)
{
    qInfo() << __FUNCTION__;
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic(T_MENU);
    menuOpenFile.setData(D_FILE_OPENDOCUMENT);
    menuOpenFile.setProperty(P_FILEPATH, filePath);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    SendEvents::navActionSwitch(dpfservice::MWNA_EDIT);
}

