// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGENGINE_H
#define DEBUGENGINE_H

#include <QSharedPointer>
#include <QObject>
#include <QTimer>

class DapSession;
class JavaDebugger;
class PythonDebugger;
class DebugEngine : public QObject
{
    Q_OBJECT
public:
    explicit DebugEngine(QObject *parent = nullptr);

    bool start();
    void stop();
    bool exit();

signals:

public slots:
private:
    bool initialize();
    void listeningIdeAlive();

    bool isRunning = false;

    QSharedPointer<DapSession> dapSession;
    QSharedPointer<JavaDebugger> javaDebugger;
    QSharedPointer<PythonDebugger> pythonDebugger;
    QTimer checkIdeAliveTimer;
};

#endif   // DEBUGENGINE_H
