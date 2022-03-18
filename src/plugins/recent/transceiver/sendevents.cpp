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
    menuOpenFile.setData("File.OpenDocument");
    menuOpenFile.setProperty("FilePath", filePath);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    navEditShow();
}

void SendEvents::recentOpenDirectory(const QString &filePath)
{
    qInfo() << __FUNCTION__;
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic(T_MENU);
    menuOpenFile.setData("File.OpenFolder");
    menuOpenFile.setProperty("FilePath", filePath);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    navEditShow();
}

void SendEvents::navEditShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavEdit;
    showNavEdit.setTopic("Nav");
    showNavEdit.setData("Edit.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavEdit);
}
