// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
