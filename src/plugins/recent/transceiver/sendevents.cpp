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

void SendEvents::recentOpenFile(const QString &filePath)
{
    qInfo() << __FUNCTION__;
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic(T_MENU);
    menuOpenFile.setData(D_FILE_OPENDOCUMENT);
    menuOpenFile.setProperty(P_FILEPATH, filePath);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    navEditShow();
}

void SendEvents::recentOpenDirectory(const QString &filePath)
{
    qInfo() << __FUNCTION__;
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic(T_MENU);
    menuOpenFile.setData(D_FILE_OPENFOLDER);
    menuOpenFile.setProperty(P_FILEPATH, filePath);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    navEditShow();
}

void SendEvents::navEditShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavEdit;
    showNavEdit.setTopic(T_NAV);
    showNavEdit.setData(D_ACTION_SWITCH);
    showNavEdit.setProperty(P_ACTION_TEXT, MWNA_EDIT);
    dpf::EventCallProxy::instance().pubEvent(showNavEdit);
}
