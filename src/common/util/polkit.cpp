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
#include "polkit.h"
#include "processutil.h"

#include <QProcess>

#define POLKIT_NAME "pkexec"

PolKit::PolKit()
{

}

PolKit &PolKit::instance()
{
    static PolKit ins;
    return ins;
}

qint64 PolKit::execute(const QString &program, const QStringList &arguments)
{
    if (!ProcessUtil::exists(program)) {
        emit failed(QString("Failed, not found %0").arg(program).toLatin1());
        return -1;
    }

    QProcess process;
    process.setProgram(POLKIT_NAME);
    QStringList argsTemp = arguments;
    argsTemp.push_front(program);
    process.setArguments(argsTemp);
    qint64 polkidID;
    process.startDetached(&polkidID);
    return polkidID;
}

void PolKit::cancel(qint64 executeID)
{
    QProcess killProcess;
    killProcess.setProgram("kill");
    killProcess.setArguments({"-9", QString("%0").arg(executeID)});
    killProcess.start();

    emit canceled();
}
