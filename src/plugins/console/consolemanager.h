// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include "consolewidget.h"

class ConsoleManagerPrivate;
class ConsoleManager : public QWidget
{
public:
    ConsoleManager(QWidget *parent = nullptr);
    ~ConsoleManager();

    QTermWidget *getCurrentConsole();
    void executeCommand(const QString &text);

private:
    void initDefaultConsole();
    void showEvent(QShowEvent *event) override;

    ConsoleManagerPrivate *d;
};

#endif   // CONSOLEMANAGER_H
