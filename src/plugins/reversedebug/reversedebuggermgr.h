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
#ifndef REVERSEDEBUGGERMGR_H
#define REVERSEDEBUGGERMGR_H

#include <QObject>

namespace ReverseDebugger {
namespace Internal {

class MinidumpRunControl;
class ReverseDebuggerMgr : public QObject
{
    Q_OBJECT
public:
    explicit ReverseDebuggerMgr(QObject *parent = nullptr);

    void initialize();

    void recored();
    void replay();

    QWidget *getWidget() const;

signals:

private slots:
    void recordMinidump();
    void runCoredump(int index);
    void unloadMinidump();

private:
    QVariant configValue(const QByteArray &name);
    void setConfigValue(const QByteArray &name, const QVariant &value);
    QString generateFilePath(const QString &fileName, const QString &traceDir, int pid);
    void replayMinidump(const QString &traceDir, int pid);
    void outputMessage(const QString &msg);
    void exist();
    QString targetPath() const;
    void enterReplayEnvironment();

    MinidumpRunControl *runCtrl = nullptr;
};

} // namespace Internal
} // namespace ReverseDebugger

#endif // REVERSEDEBUGGERMGR_H
