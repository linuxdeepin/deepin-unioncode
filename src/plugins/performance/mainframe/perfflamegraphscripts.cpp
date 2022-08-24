/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "perfflamegraphscripts.h"
#include "config.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QQueue>
#include <QDesktopServices>

namespace {

QString flameGraphPath()
{
    static QString flameGraph{"FlameGraph"};
    if (CustomPaths::installed())
        return CustomPaths::global(CustomPaths::Scripts) + QDir::separator() + flameGraph;
    else {
        return QString(CMAKE_SOURCE_DIR) + QDir::separator() + "3rdparty" + QDir::separator() + flameGraph;
    }
}

QString cachePath()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    QString appName = QCoreApplication::applicationName();

    if (!dir.exists()) {
        dir.cdUp();
        dir.mkdir(appName);
        dir.cd(appName);
    }

    if (!dir.cd(PROJECT_NAME)) {
        dir.mkdir(PROJECT_NAME);
        dir.cd(PROJECT_NAME);
    }

    return dir.path();
}

};

PerfRecord::PerfRecord(const QString &perfRecordOutFile)
    : pid (0)
    , ouFile (perfRecordOutFile)
{
    setProgram("perf");
}

PerfScript::PerfScript(const QString &perfRecordOutFileIn, const QString &outFile)
{
    setProgram("perf");
    setArguments({"script", "-i", perfRecordOutFileIn});
    setStandardOutputFile(outFile);
    QObject::connect(this, &QProcess::errorOccurred, [=](QProcess::ProcessError error){
        qCritical() << error << errorString();
    });
}

StackCollapse::StackCollapse(const QString &perfScriptOutFile, const QString &outFile)
{
    setWorkingDirectory(flameGraphPath());
    setProgram("perl");
    setArguments({"./stackcollapse-perf.pl", perfScriptOutFile});
    setStandardOutputFile(outFile);
    QObject::connect(this, &QProcess::errorOccurred, [=](QProcess::ProcessError error){
        qCritical() << error << errorString();
    });
}

FlameGraph::FlameGraph(const QString &stackCollapseOutFile, const QString &outFile)
{
    setWorkingDirectory(flameGraphPath());
    setProgram("perl");
    setArguments({"./flamegraph.pl", stackCollapseOutFile});
    setStandardOutputFile(outFile);
    QObject::connect(this, &QProcess::errorOccurred, [=](QProcess::ProcessError error){
        qCritical() << error << errorString();
    });
}


class FlameGraphGenTaskPrivate
{
    friend class FlameGraphGenTask;
    PerfRecord *perfRecord{nullptr};
    PerfScript *perfScript{nullptr};
    StackCollapse *stackCollapse{nullptr};
    FlameGraph *flameGraph{nullptr};
    QString perfRecordOutFile{cachePath() + QDir::separator() + "perfRecordOut.data"};
    QString perfScriptOutFile{cachePath() + QDir::separator() + "perfScriptOut.data"};
    QString stackCollapseOutFile{cachePath() + QDir::separator() + "stackCollapseOut.data"};
    QString flameGraphOutFile{cachePath() + QDir::separator() + "flameGraphOut.svg"};
    bool showWebBrowserFlag{false};
};

FlameGraphGenTask::FlameGraphGenTask(QObject *parent)
    : QObject (parent)
    , d (new FlameGraphGenTaskPrivate)
{
    d->perfRecord = new PerfRecord(d->perfRecordOutFile);
    d->perfScript = new PerfScript(d->perfRecordOutFile, d->perfScriptOutFile);
    d->stackCollapse = new StackCollapse(d->perfScriptOutFile, d->stackCollapseOutFile);
    d->flameGraph = new FlameGraph(d->stackCollapseOutFile, d->flameGraphOutFile);


    // perf recode exit
    QObject::connect(d->perfRecord, &QProcess::errorOccurred,
                     this, [=](QProcess::ProcessError pError)
    {
        QString errorOut = d->perfRecord->readAllStandardError();
        if (pError == QProcess::ProcessError::Crashed
                && errorOut.contains("[ perf record: Woken up")
                && errorOut.contains("times to write data ]\n[ perf record: Captured and wrote")
                && errorOut.contains("samples) ]\n")) {
            d->perfScript->start();
        } else {
            emit error(d->perfScript->program()
                       + " " + d->perfScript->arguments().join(" ")
                       + ": " + d->perfScript->errorString());
        }
    });

    QObject::connect(d->perfRecord, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int exitCode, QProcess::ExitStatus status)
    {
        Q_UNUSED(status)
        if (exitCode == 0 && d->perfScript) {
            d->perfScript->start();
        } else {
            emit error(d->perfScript->program()
                       + " " + d->perfScript->arguments().join(" ")
                       + ": " + d->perfScript->errorString());
        }
    });

    // perf script exit
    QObject::connect(d->perfScript, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int exitCode, QProcess::ExitStatus status)
    {
        Q_UNUSED(status)
        if (exitCode == 0 && d->stackCollapse) {
            d->stackCollapse->start();
        } else {
            emit error(d->perfScript->program()
                       + " " + d->perfScript->arguments().join(" ")
                       + ": " + d->perfScript->errorString());
        }
    });

    // stackCollapse exit
    QObject::connect(d->stackCollapse,  QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int exitCode, QProcess::ExitStatus status)
    {
        Q_UNUSED(status)
        if (exitCode == 0 && d->flameGraph) {
            d->flameGraph->start();
        } else {
            emit error(d->stackCollapse->program()
                       + " " + d->stackCollapse->arguments().join(" ")
                       + ": " + d->stackCollapse->errorString());
        }
    });

    // flameGraph exit
    QObject::connect(d->flameGraph,  QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int exitCode, QProcess::ExitStatus status)
    {
        Q_UNUSED(status)
        if (exitCode == 0) {
            if (d->showWebBrowserFlag) {
                QString defaultWebCmd = "gnome-www-browser";
                QProcess::startDetached(defaultWebCmd, {d->flameGraphOutFile});
                emit this->showed(d->flameGraphOutFile);
            }
        } else {
            emit error(d->flameGraph->program()
                       + " " + d->flameGraph->arguments().join(" ")
                       + ": " + d->flameGraph->errorString());
        }
    });
}

FlameGraphGenTask::~FlameGraphGenTask()
{
    if (d->flameGraph) {
        if (d->flameGraph->isReadable()) {
            d->flameGraph->kill();
            d->flameGraph->waitForFinished();
        }
        delete d->flameGraph;
    }

    if (d->stackCollapse) {
        if (d->stackCollapse->isReadable()) {
            d->stackCollapse->kill();
            d->stackCollapse->waitForFinished();
        }
        delete d->stackCollapse;
    }

    if (d->perfScript) {
        if (d->perfScript->isReadable()) {
            d->perfScript->kill();
            d->perfScript->waitForFinished();
        }
        delete d->perfScript;
    }

    if (d->perfRecord) {
        if (d->perfRecord->isReadable()) {
            d->perfRecord->kill();
            d->perfRecord->waitForFinished();
        }
        delete d->perfRecord;
    }
}

void FlameGraphGenTask::showWebBrowser(bool flag)
{
    d->showWebBrowserFlag = flag;
}

void FlameGraphGenTask::start(uint pid)
{
    if (d->perfRecord) {
        d->perfRecord->setAttachPid(pid);
        d->perfRecord->start();
        qInfo() << d->perfRecord->program() << d->perfRecord->arguments();
    }
}

void FlameGraphGenTask::stop()
{
    if (d->perfRecord && d->perfRecord->isReadable()) {
        d->perfRecord->terminate();
        d->perfRecord->waitForFinished();
    }
}

void PerfRecord::setAttachPid(uint pid)
{
    this->pid = pid;
    setArguments({"record", "-g", "-e", "cpu-clock", "-p", QString::number(pid), "-o", ouFile});
}
