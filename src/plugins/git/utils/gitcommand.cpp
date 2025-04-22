// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitcommand.h"

#include "common/util/qtcassert.h"

#include <QProcess>

#include <memory>

class GitCommandPrivate : public QObject
{
public:
    struct Job
    {
        QString program;
        QStringList arguments;
        QString workspace;
    };

    explicit GitCommandPrivate(GitCommand *qq, const QString &workspace);

    void setupProcess(QProcess *process, const Job &job);
    void startAll();
    void startNextJob();

    void processDone();
    void readStandardOutput();
    void readStandardError();

public:
    GitCommand *q;

    const QString workspace;
    QList<Job> jobList;

    std::unique_ptr<QProcess> process;
    int currentJob = 0;
    QStringList stdOut;
    QString stdErr;
};

GitCommandPrivate::GitCommandPrivate(GitCommand *qq, const QString &workspace)
    : q(qq),
      workspace(workspace)
{
}

void GitCommandPrivate::setupProcess(QProcess *process, const Job &job)
{
    process->setArguments(job.arguments);
    if (!job.workspace.isEmpty())
        process->setWorkingDirectory(job.workspace);
    process->setProgram(job.program);
}

void GitCommandPrivate::startAll()
{
    QTC_ASSERT(!jobList.isEmpty(), return );
    QTC_ASSERT(!process, return );

    currentJob = 0;
    startNextJob();
}

void GitCommandPrivate::startNextJob()
{
    QTC_ASSERT(currentJob < jobList.count(), return );

    process.reset(new QProcess);
    connect(process.get(), &QProcess::readyReadStandardOutput, this, &GitCommandPrivate::readStandardOutput);
    connect(process.get(), &QProcess::readyReadStandardError, this, &GitCommandPrivate::readStandardError);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    connect(process.get(), qOverload<int>(&QProcess::finished), this, &GitCommandPrivate::processDone);
#else
    connect(process.get(), &QProcess::finished, this, &GitCommandPrivate::processDone);
#endif
    setupProcess(process.get(), jobList.at(currentJob));
    process->start();
}

void GitCommandPrivate::processDone()
{
    ++currentJob;
    const bool success = process->exitCode() == 0;
    if (currentJob < jobList.count() && success) {
        process.release()->deleteLater();
        startNextJob();
        return;
    }

    emit q->finished(process->exitCode());
}

void GitCommandPrivate::readStandardOutput()
{
    stdOut << process->readAllStandardOutput();
}

void GitCommandPrivate::readStandardError()
{
    stdErr += process->readAllStandardError();
}

GitCommand::GitCommand(const QString &workspace, QObject *parent)
    : QObject(parent),
    d(new GitCommandPrivate(this, workspace))
{
}

GitCommand::~GitCommand()
{
    delete d;
}

void GitCommand::addJob(const QString &program, const QStringList &arguments)
{
    d->jobList.append({ program, arguments, d->workspace });
}

void GitCommand::start()
{
    if (d->jobList.isEmpty())
        return;

    d->startAll();
}

void GitCommand::cancel()
{
    if (!d->process)
        return;

    d->process->kill();
    d->process->waitForFinished();
    d->process.reset();
}

QStringList GitCommand::cleanedStdOut() const
{
    return d->stdOut;
}

QString GitCommand::cleanedStdErr() const
{
    return d->stdErr;
}
