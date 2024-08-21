// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "minidumpruncontrol.h"
#include "reversedebuggerconstants.h"
#include "common/util/custompaths.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QDir>

#include <string>
#include <unistd.h>

using namespace std;

// global variable used extern.
bool kEmdRunning = false;
bool g_emd_buffer_syscall = false;
QString g_emd_params;

namespace ReverseDebugger {
namespace Internal {

string found_crash(const char *subdir, int *ppid)
{
    // find crash.txt
    string parent_dir = getenv("HOME");
    parent_dir += subdir;   // "/.local/share/rdb/";
    string linkname = parent_dir;
    linkname += "latest-trace";
    string filename = parent_dir;
    int pos = filename.size();
    filename.resize(512, 0);
    int len = readlink(linkname.data(),
                       (char *)filename.data() + pos, 512 - pos);
    if (len < 0) {
        return string();
    }
    filename.resize(len + pos);
    parent_dir = filename;
    filename += "/crash.txt";

    // parse crash.txt
    QFile file(QString::fromStdString(filename));
    if (file.size() > 0 && file.open(QFile::ReadOnly)) {
        char buf[256];
        int size = file.readLine(buf, sizeof(buf));
        buf[size] = 0;
        char *stop = nullptr;
        int pid = strtol(buf, &stop, 10);
        int sig = strtol(stop + 1, NULL, 10);
        if (sig > 0 && pid > 0) {
            qDebug() << __FUNCTION__ << "found crash sig: " << sig;
            *ppid = pid;
            return parent_dir;
        }
    }

    return string();
}

MinidumpRunControl::MinidumpRunControl(QObject *parent)
    : QObject(parent),
      process(new QProcess(this))
{
    if (kEmdRunning) {
        qDebug() << "emd is running now!";
        return;
    }

    //    process->setWorkingDirectory()
    //    process->setProcessEnvironment();

    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(onStraceExit(int, QProcess::ExitStatus)));
}

MinidumpRunControl::~MinidumpRunControl()
{
}

void MinidumpRunControl::start(const QString &params, const QString &target)
{
    qDebug() << __FUNCTION__ << ", object:" << this;

    if (target.isEmpty() || !QFile::exists(target)) {
        QMessageBox::warning(nullptr, tr("Reverse debug"), tr("Target: %1 not found, recored failed!").arg(target));
        return;
    }

    execFile = CustomPaths::global(CustomPaths::Tools) + QDir::separator() + "emd";
    if (!params.isEmpty()) {
        execFile += ' ' + params + ' ';
    }
    execFile += target;   // target debuggee

    appendMessage(tr("[Start] %1").arg(execFile) + QLatin1Char('\n'));

    process->start(execFile);
    if (!process->waitForStarted(1000)) {
        qDebug() << "Failed to run emd";
        return;
    }

    kEmdRunning = true;
}

StopResult MinidumpRunControl::stop()
{
    qDebug() << __FUNCTION__ << ", object:" << this;

    if (process) {
        QByteArray data = process->readAll();
        QString outstr = QString::fromLocal8Bit(data.data());
        appendMessage(outstr + QLatin1Char('\n'));
    }

    kEmdRunning = false;

    appendMessage(tr("[Stop] %1").arg(execFile) + QLatin1Char('\n'));

    return StoppedSynchronously;
}

bool MinidumpRunControl::isRunning() const
{
    return kEmdRunning;
}

QString MinidumpRunControl::displayName() const
{
    return "event debug recored";
}

void MinidumpRunControl::appendMessage(const QString &msg)
{
    Q_UNUSED(msg)
}

void MinidumpRunControl::onStraceExit(int, QProcess::ExitStatus)
{
    stop();

    int pid = 0;
    string parent_dir = found_crash(("/.local/share/emd/"), &pid);
    if (!parent_dir.empty()) {
        //        emd_replay(QString::fromStdString(parent_dir), pid);
        return;
    }

    QMessageBox::information(nullptr, tr("Event Recorder"), tr("Recored done, minidump load ready."));
}

}   // namespace Internal
}   // namespace ReverseDebugger
