// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "util.h"

#include <QProcess>

bool Util::checkPackageValid(const QString &package)
{
    QProcess process;
    QString cmd("pip3 show %1");
    process.start(cmd.arg(package));
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    return !output.isEmpty();
}

void Util::installPackage(const QString &package)
{
    QProcess process;
    QString cmd("pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple %1");
    process.start(cmd.arg(package));
    process.waitForFinished();
}
