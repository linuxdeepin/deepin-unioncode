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
#include "processdialog.h"

#include <QDebug>

ProcessDialog::ProcessDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog (parent, f)
    , progressBar(new QProgressBar)
    , textBrowser(new QTextBrowser)
    , vLayout(new QVBoxLayout)
{
    setWindowTitle(__FUNCTION__);
    setMinimumSize(600, 400);
    setAttribute(Qt::WA_DeleteOnClose);

    vLayout->addWidget(textBrowser);
    vLayout->addWidget(progressBar);
    setLayout(vLayout);

    setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, &ProcessDialog::doRequestFinished);
    QObject::connect(&process, &QProcess::readyReadStandardOutput,
                     this, [&](){
        auto data = process.readAllStandardOutput();
        this->doShowRequestError(data);
    });
    QObject::connect(&process, &QProcess::readyReadStandardError,
                     this, [&](){
        auto data = process.readAllStandardError();
        this->doShowRequestError(data);
    });
}

void ProcessDialog::setProgram(const QString &program)
{
    process.setProgram(program);
}

QString ProcessDialog::program() const
{
    return process.program();
}

void ProcessDialog::setArguments(const QStringList &args)
{
    process.setArguments(args);
}

QStringList ProcessDialog::arguments()
{
    return process.arguments();
}

void ProcessDialog::setWorkDirectory(const QString &workDir)
{
    process.setWorkingDirectory(workDir);
}

QString ProcessDialog::workDirectory() const
{
    return process.workingDirectory();
}

void ProcessDialog::doShowRequestError(const QByteArray &array)
{
    textBrowser->append(array);
}

void ProcessDialog::doShowRequestOutput(const QByteArray &array)
{
    textBrowser->append(array);
}

void ProcessDialog::doRequestFinished(int exitCode, QProcess::ExitStatus status)
{
    this->close();
    qInfo() << exitCode << status;
}

void ProcessDialog::doShowProgress(int current, int count)
{
    progressBar->setRange(0, count);
    progressBar->setValue(current);
}

void ProcessDialog::showEvent(QShowEvent *event)
{
    process.start();
    return QDialog::showEvent(event);
}
