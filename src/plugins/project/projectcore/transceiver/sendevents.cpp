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

void SendEvents::projectActived(const dpfservice::ProjectInfo &info)
{
    project.activedProject(info);
}

void SendEvents::projectCreated(const dpfservice::ProjectInfo &info)
{
    project.createdProject(info);
}

void SendEvents::projectDeleted(const dpfservice::ProjectInfo &info)
{
    project.deletedProject(info);
}

void SendEvents::collaboratorsOpenRepos(const QString &workspace)
{
    dpf::Event event;
    event.setTopic(T_COLLABORATORS);
    event.setData(D_OPEN_REPOS);
    event.setProperty(P_WORKSPACEFOLDER, workspace);
    dpf::EventCallProxy::instance().pubEvent(event);
}
