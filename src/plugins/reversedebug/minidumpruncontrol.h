// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MINIDUMPRUNCONTROL_H
#define MINIDUMPRUNCONTROL_H

#include <QObject>
#include <QProcess>

namespace ReverseDebugger {
namespace Internal {

enum StopResult {
    StoppedSynchronously, // Stopped.
    AsynchronousStop     // Stop sequence has been started
};

class MinidumpRunControl : public QObject
{
    Q_OBJECT

public:
    explicit MinidumpRunControl(QObject *obj);
    ~MinidumpRunControl();

    void start(const QString &params, const QString &target);
    StopResult stop();
    bool isRunning() const;
    QString displayName() const;

private:
    void appendMessage(const QString &msg);

    QProcess *process = nullptr;
    QString execFile;

private slots:
     void onStraceExit(int, QProcess::ExitStatus);

};

} // namespace Internal
} // namespace ReverseDebugger

#endif // MINIDUMPRUNCONTROL_H
