// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sendevents.h"
#include "framework.h"
#include "common/common.h"

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
