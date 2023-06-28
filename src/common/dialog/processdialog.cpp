// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    vLayout->addWidget(textBrowser);
    vLayout->addWidget(progressBar);
    setLayout(vLayout);

    QObject::connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, &ProcessDialog::doFinished);
    QObject::connect(&process, &QProcess::readyReadStandardOutput,
                     this, [&](){
        auto data = process.readAllStandardOutput();
        this->doShowStdOut(data);
    });
    QObject::connect(&process, &QProcess::readyReadStandardError,
                     this, [&](){
        auto data = process.readAllStandardError();
        this->doShowStdErr(data);
    });
}

ProcessDialog::~ProcessDialog()
{
    process.kill();
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

void ProcessDialog::setWorkingDirectory(const QString &workDir)
{
    process.setWorkingDirectory(workDir);
}

QString ProcessDialog::workDirectory() const
{
    return process.workingDirectory();
}

void ProcessDialog::setEnvironment(const QStringList &env)
{
    process.setEnvironment(env);
}

int ProcessDialog::exec()
{
    process.start();
    return QDialog::exec();
}

void ProcessDialog::doShowStdErr(const QByteArray &array)
{
    textBrowser->append(array);
}

void ProcessDialog::doShowStdOut(const QByteArray &array)
{
    textBrowser->append(array);
}

void ProcessDialog::doFinished(int exitCode, QProcess::ExitStatus status)
{
    this->close();
    qInfo() << exitCode << status;
}

void ProcessDialog::doShowProgress(int current, int count)
{
    progressBar->setRange(0, count);
    progressBar->setValue(current);
}
