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

#include "mavenmanager.h"

#include "services/builder/builderservice.h"
#include "services/project/projectinfo.h"
#include "services/builder/task.h"
#include "services/builder/fileutils.h"

#include <QtConcurrent>

using namespace dpfservice;

class MavenManagerPrivate
{
    friend class MavenManager;
    QScopedPointer<QProcess> process;
};

MavenManager *MavenManager::instance()
{
    static MavenManager ins;
    return &ins;
}

MavenManager::MavenManager(QObject *parent)
    : QObject(parent)
    , d(new MavenManagerPrivate())
{

}

MavenManager::~MavenManager()
{
    if (d) {
        delete d;
    }
}

void MavenManager::executeBuildCommand(const QString &program, const QStringList &arguments, const QString &workingDir)
{
    if (program.trimmed() != "mvn")
        return;

    QtConcurrent::run(this, &MavenManager::execCommand, program, arguments, workingDir);
}

bool MavenManager::execCommand(const QString &program, const QStringList &arguments, const QString &workingDir)
{
    bool ret = false;
    d->process.reset(new QProcess());
    d->process->setWorkingDirectory(workingDir);
    connect(d->process.get(), static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
        if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
            ret = true;
            emit addCompileOutput(tr("The process \"%1\" exited normally.")
                           .arg(d->process->program()), OutputFormat::NormalMessage);
        } else if (exitStatus == QProcess::NormalExit) {
            ret = false;
            emit addCompileOutput(tr("The process \"%1\" exited with code %2.")
                           .arg(d->process->program(), QString::number(exitcode)), OutputFormat::ErrorMessage);
        } else {
            ret = false;
            emit addCompileOutput(tr("The process \"%1\" crashed.")
                           .arg(d->process->program()), OutputFormat::ErrorMessage);
        }
    });

    connect(d->process.get(), &QProcess::readyReadStandardOutput,
            this, &MavenManager::processReadyReadStdOutput);

    connect(d->process.get(), &QProcess::readyReadStandardError,
            this, &MavenManager::processReadyReadStdError);

    d->process->start(program, arguments);
    d->process->waitForFinished();

    return ret;
}

void MavenManager::processReadyReadStdOutput()
{
    d->process->setReadChannel(QProcess::StandardOutput);
    while (d->process->canReadLine()) {
        QString line = QString::fromUtf8(d->process->readLine());
        emit addCompileOutput(line, OutputFormat::NormalMessage);
    }
}

void MavenManager::processReadyReadStdError()
{
    d->process->setReadChannel(QProcess::StandardError);
    while (d->process->canReadLine()) {
        QString line = QString::fromUtf8(d->process->readLine());
        emit addCompileOutput(line, OutputFormat::ErrorMessage);
        Task task(Task::Error, line, Utils::FileName(), -1, "");
        emit addProblemOutput(task, 0, 0);
    }
}

void MavenManager::buildProject()
{

}

void MavenManager::rebuildProject()
{

}

void MavenManager::cleanProject()
{

}
