/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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

#include "buildmanager.h"
#include "menumanager.h"
#include "compileoutputpane.h"
#include "problemoutputpane.h"
#include "transceiver/buildersender.h"

#include "services/builder/builderservice.h"
#include "services/project/projectinfo.h"

using namespace dpfservice;

class BuildManagerPrivate
{
    friend class BuildManager;

    QSharedPointer<MenuManager> menuManager;
    CompileOutputPane *compileOutputPane = nullptr;
    ProblemOutputPane *problemOutputPane = nullptr;
};

BuildManager *BuildManager::instance()
{
    static BuildManager ins;
    return &ins;
}

BuildManager::BuildManager(QObject *parent)
    : QObject(parent)
    , d(new BuildManagerPrivate())
{

}

BuildManager::~BuildManager()
{
    if (d) {
        delete d;
    }
}

void BuildManager::initialize(dpfservice::WindowService *windowService)
{
    d->menuManager.reset(new MenuManager(windowService));
    d->compileOutputPane = new CompileOutputPane();
    d->problemOutputPane = new ProblemOutputPane();

    QObject::connect(d->menuManager->getActionPointer(ActionType::build).get(), &QAction::triggered,
                     BuildManager::instance(), &BuildManager::buildActivedProject, Qt::DirectConnection);
    QObject::connect(d->menuManager->getActionPointer(ActionType::rebuild).get(), &QAction::triggered,
                     BuildManager::instance(), &BuildManager::rebuildActivedProject, Qt::DirectConnection);
    QObject::connect(d->menuManager->getActionPointer(ActionType::clean).get(), &QAction::triggered,
                     BuildManager::instance(), &BuildManager::cleanActivedProject, Qt::DirectConnection);

}

CompileOutputPane *BuildManager::getCompileOutputPane() const
{
    return d->compileOutputPane;
}

ProblemOutputPane *BuildManager::getProblemOutputPane() const
{
    return d->problemOutputPane;
}

void BuildManager::buildActivedProject()
{
    BuilderSender::menuBuild();
    startBuild();
}

void BuildManager::rebuildActivedProject()
{
    BuilderSender::menuReBuild();
    startBuild();
}

void BuildManager::cleanActivedProject()
{
    BuilderSender::menuClean();
    startBuild();
}

void BuildManager::dispatchCommand(const QString &program, const QStringList &arguments, const QString &workingDir)
{
    BuilderSender::sendCommand(program, arguments, workingDir);
}

void BuildManager::startBuild()
{
    d->compileOutputPane->clearContents();
    d->problemOutputPane->clearContents();
    emit buildStarted();
}

void BuildManager::outputCompileInfo(const QString &content, OutputFormat format)
{
    QString outputContent = content;
    if (format == NormalMessage) {
        QDateTime curDatetime = QDateTime::currentDateTime();
        QString time = curDatetime.toString("hh:mm:ss");
        outputContent = time + ":" + content + "\r";
    }

    d->compileOutputPane->appendText(outputContent, format);
}

void BuildManager::outputProblemInfo(const Task &task, int linkedOutputLines, int skipLines)
{
    d->problemOutputPane->addTask(task, linkedOutputLines, skipLines);
}

void BuildManager::buildStateChanged(BuildState state, QString originCmd)
{
    BuilderSender::notifyBuildState(state, originCmd);
    d->menuManager->handleRunStateChanged(state);
}
