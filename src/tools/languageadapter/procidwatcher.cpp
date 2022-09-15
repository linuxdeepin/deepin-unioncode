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
#include "procidwatcher.h"

#include <QCoreApplication>
#include <QProcess>
#include <QDebug>

ProcIdWatcher::ProcIdWatcher(unsigned parentPid)
    : QTimer()
    , parentPid(parentPid)
{
    setInterval(300);
    QObject::connect(this, &QTimer::timeout,
                     this, [=](){
        QProcess process;
        process.setProgram("ps");
        process.setArguments({"--pid", QString::number(parentPid)});
        process.start();
        process.waitForFinished();
        QByteArray queryPidLines = process.readAllStandardOutput();
        if (queryPidLines.isEmpty() || queryPidLines.count('\n') < 2) {
            qCritical() << "not found parent pid:"
                        << QString::number(parentPid)
                        << "qApp quit";
            qApp->quit();
        }
    });
    start();
}
