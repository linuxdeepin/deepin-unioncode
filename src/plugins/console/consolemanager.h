// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include "consolewidget.h"

class QProcess;
class ConsoleManagerPrivate;
class ConsoleManager : public QWidget
{
    Q_OBJECT
public:
    ConsoleManager(QWidget *parent = nullptr);
    ~ConsoleManager();

    QTermWidget *currentConsole();
    QTermWidget *findConsole(const QString &id);
    QTermWidget *createConsole(const QString &name, bool startNow = true, bool rename = true);
    void sendCommand(const QString &text);
    void executeCommand(const QString &name, const QString &program, const QStringList &args, const QString &workingDir, const QStringList &env);
    QUuid newConsole(const QString &name, bool rename = true);
    void selectConsole(const QUuid &uuid);
    void run2Console(const QUuid &uuid, const QProcess &process);

private:
    void showEvent(QShowEvent *event) override;

    ConsoleManagerPrivate *d;
};

#endif   // CONSOLEMANAGER_H
