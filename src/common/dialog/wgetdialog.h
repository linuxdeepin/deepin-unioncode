// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WGETDIALOG_H
#define WGETDIALOG_H

#include "processdialog.h"

class WGetDialog : public ProcessDialog
{
    Q_OBJECT
public:
    WGetDialog(QWidget *parent = nullptr);
    void setWorkingDirectory(const QString &workDir);
    QString workDirectory() const;
    void setWgetArguments(const QStringList &list);
    QStringList wgetArguments() const;

protected:
    virtual void doShowStdErr(const QByteArray &array) override;
    virtual void doShowStdOut(const QByteArray &array) override;
    virtual void doFinished(int exitCode, QProcess::ExitStatus status) override;
    virtual void showEvent(QShowEvent *event) override;
};

#endif // WGETDIALOG_H
