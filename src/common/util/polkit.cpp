// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
