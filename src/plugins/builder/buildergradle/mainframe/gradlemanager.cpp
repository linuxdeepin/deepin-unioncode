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

#include "gradlemanager.h"

#include "services/builder/builderservice.h"
#include "services/project/projectinfo.h"

#include <QtConcurrent>

const QString CMD_GRADLE = "gradle";
using namespace dpfservice;

class GradleManagerPrivate
{
    friend class GradleManager;
    QScopedPointer<QProcess> process;
};

GradleManager *GradleManager::instance()
{
    static GradleManager ins;
    return &ins;
}

GradleManager::GradleManager(QObject *parent)
    : QObject(parent)
    , d(new GradleManagerPrivate())
{

}

GradleManager::~GradleManager()
{
    if (d) {
        delete d;
    }
}

void GradleManager::executeBuildCommand(const QString &program, const QStringList &arguments, const QString &workingDir)
{
    if (program.trimmed() != CMD_GRADLE)
        return;

    emit buildStart();
    QtConcurrent::run(this, &GradleManager::execCommand, program, arguments, workingDir);
}

bool GradleManager::execCommand(const QString &program, const QStringList &arguments, const QString &workingDir)
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
            this, &GradleManager::processReadyReadStdOutput);

    connect(d->process.get(), &QProcess::readyReadStandardError,
            this, &GradleManager::processReadyReadStdError);

    d->process->start(program, arguments);
    d->process->waitForFinished();

    return ret;
}

void GradleManager::processReadyReadStdOutput()
{
    d->process->setReadChannel(QProcess::StandardOutput);
    while (d->process->canReadLine()) {
        QString line = QString::fromUtf8(d->process->readLine());
        emit addCompileOutput(line, OutputFormat::NormalMessage);
    }
}

void GradleManager::processReadyReadStdError()
{
    d->process->setReadChannel(QProcess::StandardError);
    while (d->process->canReadLine()) {
        QString line = QString::fromUtf8(d->process->readLine());
        emit addCompileOutput(line, OutputFormat::ErrorMessage);
        Task task(Task::Error, line, Utils::FileName(), -1, "");
        emit addProblemOutput(task, 0, 0);
    }
}

void GradleManager::buildProject()
{

}

void GradleManager::rebuildProject()
{

}

void GradleManager::cleanProject()
{

}
