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
#include "projectmavenreceiver.h"
#include "mainframe/mavenopenhandler.h"

#include "common/common.h"

QString ProjectMavenProxy::buildOriginCmdCache{};

ProjectMavenReceiver::ProjectMavenReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<ProjectMavenReceiver> ()
{

}

dpf::EventHandler::Type ProjectMavenReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ProjectMavenReceiver::topics()
{
    return { T_PROJECT, project.topic};
}

void ProjectMavenReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == project.openProject.name) {
        QString filePathKey = project.openProject.pKeys[0];
        QString kitNameKey = project.openProject.pKeys[1];
        QString languageKey = project.openProject.pKeys[2];
        QString workspaceKey = project.openProject.pKeys[3];
        CmakeOpenHandler::instance()->doProjectOpen(
                    event.property(kitNameKey).toString(),
                    event.property(languageKey).toString(),
                    event.property(filePathKey).toString());
    }

    if (event.data() == D_OPENPROJECT) {
        CmakeOpenHandler::instance()->doProjectOpen(
                    event.property(P_KITNAME).toString(),
                    event.property(P_LANGUAGE).toString(),
                    event.property(P_FILEPATH).toString());
    }
}

ProjectMavenProxy *ProjectMavenProxy::instance()
{
    static ProjectMavenProxy ins;
    return &ins;
}

void ProjectMavenProxy::setbuildOriginCmd(const QString &originCmd)
{
    buildOriginCmdCache = originCmd;
}

QString ProjectMavenProxy::buildOriginCmd()
{
    return buildOriginCmdCache;
}
