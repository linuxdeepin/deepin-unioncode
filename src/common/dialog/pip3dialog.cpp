// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "pip3dialog.h"

#include <QDebug>

Pip3Dialog::Pip3Dialog(QWidget *parent)
    : ProcessDialog (parent)
{
    setWindowTitle("pip3 execute");
    process.setProgram("pip3");
}

void Pip3Dialog::install(const QString &packageName)
{
    process.setArguments({"install", packageName});
}

void Pip3Dialog::uninstall(const QString &packageName)
{
    process.setArguments({"uninstall", packageName});
}

void Pip3Dialog::doShowStdErr(const QByteArray &array)
{
    qInfo() << array;
}
