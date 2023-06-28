// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSDIALOG_H
#define PROCESSDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QProgressBar>
#include <QTextBrowser>
#include <QVBoxLayout>

class ProcessDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProcessDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~ProcessDialog();
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
    QProgressBar *progressBar{nullptr};
    QTextBrowser *textBrowser{nullptr};
    QVBoxLayout *vLayout{nullptr};
};

#endif // PROCESSDIALOG_H
