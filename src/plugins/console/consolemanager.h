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

    QTermWidget *getCurrentConsole();
    void executeCommand(const QString &text);

private:
    void initDefaultConsole();
    void initUI();
    void initConnection();
    void appendConsole();
    void removeConsole();
    void switchConsole(const QModelIndex &index);
    void showEvent(QShowEvent *event) override;

    ConsoleManagerPrivate *d;
};

#endif   // CONSOLEMANAGER_H
