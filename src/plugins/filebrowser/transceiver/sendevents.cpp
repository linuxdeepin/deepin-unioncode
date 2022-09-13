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

void SendEvents::treeViewDoublueClicked(const QString &filePath, const dpfservice::ProjectInfo &proInfo)
{
    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_OPENFILE);
    event.setProperty(P_PROJECT_INFO, QVariant::fromValue(proInfo));
    event.setProperty(P_FILEPATH, filePath);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void SendEvents::buildProject(const QString &buildSystem,
                              const QString &buildDir,
                              const QString &buildFilePath,
                              const QStringList &buildArgs)
{
    dpf::Event event;
    event.setTopic(T_FILEBROWSER);
    event.setData(D_ITEM_MENU_BUILD);
    event.setProperty(P_BUILDSYSTEM, buildSystem);
    event.setProperty(P_BUILDDIRECTORY, buildDir);
    event.setProperty(P_BUILDFILEPATH, buildFilePath);
    event.setProperty(P_BUILDARGUMENTS, buildArgs);
    qInfo() << __FUNCTION__ << event;
    dpf::EventCallProxy::instance().pubEvent(event);
}

void SendEvents::gengrateWorkspace(const QString &projectPath)
{
    dpf::Event event;
    event.setTopic(T_FILEBROWSER);
    event.setData(D_ITEM_MENU_BUILD);
    event.setProperty(P_PROJECTPATH, projectPath);
    qInfo() << __FUNCTION__ << event;
    dpf::EventCallProxy::instance().pubEvent(event);
}
