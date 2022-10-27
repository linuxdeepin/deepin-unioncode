/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
