/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
    void setWorkDirectory(const QString &workDir);
    QString workDirectory() const;
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
