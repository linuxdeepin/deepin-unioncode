/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<huangyub@uniontech.com>
 *
 * Maintainer: hongjinchuan<huangyub@uniontech.com>
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

#include "valgrindreceiver.h"
#include "mainframe/valgrindrunner.h"

#include "services/project/projectinfo.h"
#include "services/project/projectservice.h"
#include "services/window/windowelement.h"
#include "services/project/projectgenerator.h"
#include "services/builder/builderglobals.h"
#include "common/common.h"

ValgrindReceiver::ValgrindReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<ValgrindReceiver> ()
{

}

dpf::EventHandler::Type ValgrindReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ValgrindReceiver::topics()
{
    return {project.topic, editor.topic};
}

void ValgrindReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == project.activedProject.name) {
        QVariant proInfoVar = event.property(project.activedProject.pKeys[0]);
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        QString buildFolder = projectInfo.buildFolder();
        ValgrindRunner::instance()->saveCurrentProjectInfo(projectInfo);
    } else if (event.data() == project.createdProject.name) {
        QVariant proInfoVar = event.property(project.createdProject.pKeys[0]);
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        QString buildFolder = projectInfo.buildFolder();
        ValgrindRunner::instance()->saveCurrentProjectInfo(projectInfo);
    } else if (event.data() == project.deletedProject.name) {
        QVariant proInfoVar = event.property(project.deletedProject.pKeys[0]);
        dpfservice::ProjectInfo projectInfo = qvariant_cast<dpfservice::ProjectInfo>(proInfoVar);
        ValgrindRunner::instance()->removeProjectInfo();
    }  else if (event.data() == editor.switchedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        ValgrindRunner::instance()->saveCurrentFilePath(filePath);
    } else if (event.data() == editor.openedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        ValgrindRunner::instance()->saveCurrentFilePath(filePath);
    } else if (event.data() == editor.closedFile.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        ValgrindRunner::instance()->removeCurrentFilePath();
    }
}
