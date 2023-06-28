// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REVERSEDEBUGGERMGR_H
#define REVERSEDEBUGGERMGR_H

#include <QObject>

class QSettings;
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
    bool replayMinidump(const QString &traceDir, int pid);
    void outputMessage(const QString &msg);
    void exist();
    const QString &dumpTargetPath() const;
    QString projectTargetPath() const;
    void enterReplayEnvironment();

    MinidumpRunControl *runCtrl = nullptr;
    QSettings *settings = nullptr;
    QString targetPath;
};

} // namespace Internal
} // namespace ReverseDebugger

#endif // REVERSEDEBUGGERMGR_H
