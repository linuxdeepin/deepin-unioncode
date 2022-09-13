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
#include "services/window/windowservice.h"

void SendEvents::recentOpenFile(const QString &filePath)
{
    qInfo() << __FUNCTION__;
    dpf::Event recentOpenFile;
    recentOpenFile.setTopic(T_PROJECT);
    recentOpenFile.setData(D_OPENDOCUMENT);
    recentOpenFile.setProperty(P_FILEPATH, filePath);
    dpf::EventCallProxy::instance().pubEvent(recentOpenFile);
    navEditShow();
}

void SendEvents::recentOpenProject(const QString &filePath,
                                   const QString &kitName,
                                   const QString &language,
                                   const QString &workspace)
{
    qInfo() << __FUNCTION__;
    dpf::Event recentOpenFile;
    recentOpenFile.setTopic(T_PROJECT);
    recentOpenFile.setData(D_OPENPROJECT);
    recentOpenFile.setProperty(P_FILEPATH, filePath);
    recentOpenFile.setProperty(P_KITNAME, kitName);
    recentOpenFile.setProperty(P_LANGUAGE, language);
    recentOpenFile.setProperty(P_WORKSPACEFOLDER, workspace);
    dpf::EventCallProxy::instance().pubEvent(recentOpenFile);
    navEditShow();
}

void SendEvents::navEditShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavEdit;
    showNavEdit.setTopic(T_NAV);
    showNavEdit.setData(D_ACTION_SWITCH);
    showNavEdit.setProperty(P_ACTION_TEXT, dpfservice::MWNA_EDIT);
    dpf::EventCallProxy::instance().pubEvent(showNavEdit);
}
