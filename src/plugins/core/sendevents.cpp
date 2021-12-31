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

#include <framework/framework.h>

void SendEvents::navRecentShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavRecent;
    showNavRecent.setTopic("Nav");
    showNavRecent.setData("Recent.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavRecent);
}

void SendEvents::navRecentHide()
{
    qInfo() << __FUNCTION__;
    dpf::Event hideNavRecent;
    hideNavRecent.setTopic("Nav");
    hideNavRecent.setData("Recent.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavRecent);
}

void SendEvents::navEditShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavEdit;
    showNavEdit.setTopic("Nav");
    showNavEdit.setData("Edit.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavEdit);
}

void SendEvents::navEditHide()
{
    qInfo() << __FUNCTION__;
    dpf::Event hideNavEdit;
    hideNavEdit.setTopic("Nav");
    hideNavEdit.setData("Edit.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavEdit);
}

void SendEvents::navDebugShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavDebug;
    showNavDebug.setTopic("Nav");
    showNavDebug.setData("Debug.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavDebug);
}

void SendEvents::NavDebugHide()
{
    qInfo() << __FUNCTION__;
    dpf::Event hideNavDebug;
    hideNavDebug.setTopic("Nav");
    hideNavDebug.setData("Debug.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavDebug);
}

void SendEvents::navRuntimeShow()
{
    qInfo() << __FUNCTION__;
    dpf::Event showNavRuntime;
    showNavRuntime.setTopic("Nav");
    showNavRuntime.setData("Runtime.Show");
    dpf::EventCallProxy::instance().pubEvent(showNavRuntime);
}

void SendEvents::navRuntimeHide()
{
    qInfo() << __FUNCTION__;
    dpf::Event hideNavRuntime;
    hideNavRuntime.setTopic("Nav");
    hideNavRuntime.setData("Runtime.Hide");
    dpf::EventCallProxy::instance().pubEvent(hideNavRuntime);
}

void SendEvents::menuOpenFile(const QString &filePath)
{
    qInfo() << __FUNCTION__;
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic("Menu");
    menuOpenFile.setData("File.OpenDocument");
    menuOpenFile.setProperty("FilePath", filePath);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    navEditShow();
}

void SendEvents::menuOpenDirectory(const QString &filePath)
{
    qInfo() << __FUNCTION__;
    dpf::Event menuOpenFile;
    menuOpenFile.setTopic("Menu");
    menuOpenFile.setData("File.OpenFolder");
    menuOpenFile.setProperty("FilePath", filePath);
    dpf::EventCallProxy::instance().pubEvent(menuOpenFile);
    navEditShow();
}
