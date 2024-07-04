// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include "consolewidget.h"

class ConsoleManagerPrivate;
class ConsoleManager : public QWidget
{
    Q_OBJECT
public:
    ConsoleManager(QWidget *parent = nullptr);
    ~ConsoleManager();

    QTermWidget *currentConsole();
    QTermWidget *findConsole(const QString &id);
    QTermWidget *createConsole(const QString &name, bool startNow = true);
    void sendCommand(const QString &text);
    void executeCommand(const QString &name, const QString &program, const QStringList &args, const QString &workingDir, const QStringList &env);

private:
    void showEvent(QShowEvent *event) override;

    ConsoleManagerPrivate *d;
};

#endif   // CONSOLEMANAGER_H
