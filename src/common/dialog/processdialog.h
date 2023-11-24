// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <DAbstractDialog>
#include <DProgressBar>
#include <DTextBrowser>

#include <QProcess>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class ProcessDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit ProcessDialog(QWidget *parent = nullptr);
    virtual ~ProcessDialog() override;
    void setProgram(const QString & program);
    QString program() const;
    void setArguments(const QStringList &args);
    QStringList arguments();
    void setWorkingDirectory(const QString &workDir);
    QString workDirectory() const;
    void setEnvironment(const QStringList &env);
    virtual int exec() override;

protected:
    virtual void doShowStdErr(const QByteArray &array);
    virtual void doShowStdOut(const QByteArray &array);
    virtual void doFinished(int exitCode, QProcess::ExitStatus status);
    virtual void doShowProgress(int current, int count);

protected:
    QProcess process;
    DProgressBar *progressBar{nullptr};
    DTextBrowser *textBrowser{nullptr};
    QVBoxLayout *vLayout{nullptr};
};

#endif // PROCESSDIALOG_H
