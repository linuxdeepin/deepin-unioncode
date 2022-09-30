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
#include "projectgradlereceiver.h"
#include "mainframe/gradleopenhandler.h"

#include "common/common.h"

ProjectGradleReceiver::ProjectGradleReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<ProjectGradleReceiver> ()
{

}

dpf::EventHandler::Type ProjectGradleReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ProjectGradleReceiver::topics()
{
    return { T_PROJECT, project.topic};
}

void ProjectGradleReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == project.openProject.name) {
        QString filePathKey = project.openProject.pKeys[0];
        QString kitNameKey = project.openProject.pKeys[1];
        QString languageKey = project.openProject.pKeys[2];
        QString workspaceKey = project.openProject.pKeys[3];
        GradleOpenHandler::instance()->doProjectOpen(
                    event.property(kitNameKey).toString(),
                    event.property(languageKey).toString(),
                    event.property(filePathKey).toString());
    }

    if (event.data() == D_OPENPROJECT) {
        GradleOpenHandler::instance()->doProjectOpen(
                    event.property(P_KITNAME).toString(),
                    event.property(P_LANGUAGE).toString(),
                    event.property(P_FILEPATH).toString());
    }
}

ProjectGradleProxy *ProjectGradleProxy::instance()
{
    static ProjectGradleProxy ins;
    return &ins;
}
