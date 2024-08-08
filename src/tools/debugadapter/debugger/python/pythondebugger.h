// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PYTHONDEBUGGER_H
#define PYTHONDEBUGGER_H


#include <QObject>

class PythonDebuggerPrivate;
class PythonDebugger : public QObject
{
    Q_OBJECT
public:
    explicit PythonDebugger(QObject *parent = nullptr);
    virtual ~PythonDebugger() override;

signals:
    void sigSendToClient(const QString &ppid, int port, const QString &kit, const QString &projectPath);

public slots:
    void slotReceiveClientInfo(const QString &ppid,
                               const QString &kit,
                               const QString &pythonExecute,
                               const QString &fileName,
                               const QString &projectPath,
                               const QString &projectCachePath);

private:
    void registerLaunchDAPConnect();
    void initialize(const QString &pythonExecute,
                    const QString &fileName,
                    const QString &projectCachePath);

    PythonDebuggerPrivate *const d;
};

#endif // PYTHONDEBUGGER_H
