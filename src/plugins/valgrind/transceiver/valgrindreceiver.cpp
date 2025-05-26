// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "valgrindreceiver.h"
#include "mainframe/valgrindrunner.h"
#include "common/common.h"

#include "services/project/projectservice.h"
#include "services/window/windowelement.h"
#include "services/project/projectgenerator.h"
#include "services/builder/builderglobals.h"

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
    if (event.data() == project.activatedProject.name) {
        QVariant proInfoVar = event.property(project.activatedProject.pKeys[0]);
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
    } else if (event.data() == editor.gotoLine.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        ValgrindRunner::instance()->saveCurrentFilePath(filePath);
    } else if (event.data() == editor.fileClosed.name) {
        QString filePath = event.property(editor.switchedFile.pKeys[0]).toString();
        ValgrindRunner::instance()->removeCurrentFilePath();
    }
}
