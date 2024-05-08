// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GITCOMMAND_H
#define GITCOMMAND_H

#include <QObject>

class GitCommandPrivate;
class GitCommand : public QObject
{
    Q_OBJECT
public:
    explicit GitCommand(const QString &workspace, QObject *parent = nullptr);
    ~GitCommand() override;

    void addJob(const QString &program, const QStringList &arguments);
    void start();
    void cancel();

    QStringList cleanedStdOut() const;
    QString cleanedStdErr() const;

Q_SIGNALS:
    void finished(int code);

private:
    class GitCommandPrivate *const d;
};

#endif   // GITCOMMAND_H
