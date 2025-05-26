// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDEXECUTER_H
#define COMMANDEXECUTER_H

#include <framework/framework.h>
#include <QObject>

class CommandExecuter: QObject
{
    Q_OBJECT
public:
    static CommandExecuter &instance();
    void buildProject();

private:
    explicit CommandExecuter(QObject *parent = nullptr);
    QStringList exeFileName(QDir dir);

};

#endif   // COMMANDEXECUTER_H
