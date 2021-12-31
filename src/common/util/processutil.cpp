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
#include "processutil.h"

#include <QProcess>
#include <QFileInfo>
#include <QDebug>

bool ProcessUtil::execute(const QString &program,
                          const QStringList &arguments,
                          ProcessUtil::ReadCallBack func)
{
    bool ret = false;
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.connect(&process, QOverload<int, QProcess::ExitStatus >::of(&QProcess::finished),
                    [&ret, &process](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitCode == 0 && exitStatus == QProcess::NormalExit)
            ret = true;
        qInfo() << process.program() << process.arguments();
    });

    if (func) {
        QProcess::connect(&process, &QProcess::readyRead, [&process, &func](){
            func(process.readAll());
        });
    }
    process.start();
    process.waitForFinished();

    return ret;
}

bool ProcessUtil::exists(const QString &name)
{
    bool ret = false;
#ifdef linux
    auto outCallback = [&ret, &name](const QByteArray &array) {
        QList<QByteArray> rmSearch = array.split(' ');
        foreach (QByteArray rmProcess, rmSearch) {
            QFileInfo info(rmProcess);
            if (info.fileName() == name && info.isExecutable()) {
                if (!ret)
                    ret = true;
            }
        }
    };
    ret = ProcessUtil::execute("whereis", {name}, outCallback);
#else
#endif
    return ret;
}

QString ProcessUtil::version(const QString &name)
{
    QString retOut;
#ifdef linux
    auto outCallback = [&retOut](const QByteArray &array) {
        retOut = QString ::fromLatin1(array);
    };
    ProcessUtil::execute(name, {"-version"}, outCallback);
#else
#endif
    return retOut;
}

bool ProcessUtil::hasGio()
{
    return exists("gio");
}

bool ProcessUtil::moveToTrash(const QString &filePath)
{
#ifdef linux
    if (!hasGio())
        return false;
    return ProcessUtil::execute("gio", {"trash", filePath});
#else

#endif
}
