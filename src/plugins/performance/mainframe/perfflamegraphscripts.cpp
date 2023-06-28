// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
            qInfo() << "start perfScript with crashed perfRecord";
            d->perfScript->start();
        } else {
            emit error(d->perfScript->program()
                       + " " + d->perfScript->arguments().join(" ")
                       + ": " + d->perfScript->errorString());
        }
    });

    QObject::connect(d->perfRecord, &QProcess::readAllStandardOutput,
                     this, [=](){
        qCritical() << "perfRecord output: \n"
                    << d->perfRecord->readAllStandardOutput();
    });
    QObject::connect(d->perfRecord, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int exitCode, QProcess::ExitStatus status)
    {
        qInfo() << "perfRecord exit:" << exitCode << status;
        if (exitCode == 0 && d->perfScript) {
            qInfo() << "start perfScript";
            d->perfScript->start();
        } else {
            qCritical() << "exit not's 0, this unknow error from perfRecord"
                        << d->perfScript->errorString();
            emit error(d->perfScript->program()
                       + " " + d->perfScript->arguments().join(" ")
                       + ": " + d->perfScript->errorString());
        }
    });

    QObject::connect(d->perfScript, &QProcess::readAllStandardOutput,
                     this, [=](){
        qCritical() << "perfScript output: \n"
                    << d->perfScript->readAllStandardOutput();
    });
    QObject::connect(d->perfScript, &QProcess::readyReadStandardError,
                     this, [=]()
    {
        qCritical() << "perfScript error output: \n"
                    << d->perfScript->readAllStandardError();
    });
    // perf script exit
    QObject::connect(d->perfScript, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int exitCode, QProcess::ExitStatus status)
    {
        qInfo() << "perfScript exit:" << exitCode << status;
        if (exitCode == 0 && d->stackCollapse) {
            qInfo() << "start stackCollapse script";
            d->stackCollapse->start();
        } else {
            qCritical() << "exit not's 0, this unknow error from perfScript"
                        << d->perfScript->errorString();
            emit error(d->perfScript->program()
                       + " " + d->perfScript->arguments().join(" ")
                       + ": " + d->perfScript->errorString());
        }
    });

    QObject::connect(d->stackCollapse, &QProcess::readAllStandardOutput,
                     this, [=](){
        qCritical() << "stackCollapse output: \n"
                    << d->stackCollapse->readAllStandardOutput()
                    << d->stackCollapse->workingDirectory()
                    << d->stackCollapse->program()
                    << d->stackCollapse->arguments();
    });
    QObject::connect(d->stackCollapse, &QProcess::readyReadStandardError,
                     this, [=]()
    {
        qCritical() << "stackCollapse error output: \n"
                    << d->stackCollapse->readAllStandardError()
                    << d->stackCollapse->workingDirectory()
                    << d->stackCollapse->program()
                    << d->stackCollapse->arguments();
    });
    // stackCollapse exit
    QObject::connect(d->stackCollapse, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int exitCode, QProcess::ExitStatus status)
    {
        qInfo() << "stackCollapse exit:" << exitCode << status;
        if (exitCode == 0 && d->flameGraph) {
            qCritical() << "start flameGraph script";
            d->flameGraph->start();
        } else {
            qCritical() << "exit not's 0, this unknow error from stackCollapse"
                        << d->flameGraph->errorString();
            emit error(d->stackCollapse->program()
                       + " " + d->stackCollapse->arguments().join(" ")
                       + ": " + d->stackCollapse->errorString());
        }
    });

    QObject::connect(d->flameGraph, &QProcess::readAllStandardOutput,
                     this, [=](){
        qCritical() << "flameGraph output: \n"
                    << d->flameGraph->readAllStandardOutput();
    });
    QObject::connect(d->flameGraph, &QProcess::readyReadStandardError,
                     this, [=]()
    {
        qCritical() << "flameGraph error output: \n"
                    << d->flameGraph->readAllStandardError();
    });
    // flameGraph exit
    QObject::connect(d->flameGraph, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [=](int exitCode, QProcess::ExitStatus status)
    {
        Q_UNUSED(status)
        qInfo() << "stackCollapse exit:" << exitCode << status;
        if (exitCode == 0) {
            if (d->showWebBrowserFlag) {
                qCritical() << "show with gnome-www-browser";
                QString defaultWebCmd = "gnome-www-browser";
                QProcess::startDetached(defaultWebCmd, {d->flameGraphOutFile});
                emit this->showed(d->flameGraphOutFile);
            }
        } else {
            qCritical() << "exit not's 0, this unknow error from flameGraph"
                        << d->flameGraph->errorString();
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
