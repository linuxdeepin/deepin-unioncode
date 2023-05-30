/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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

#include "binarytoolsdialog.h"
#include "binarytoolsconfigview.h"
#include "common/util/eventdefinitions.h"

#include <QPushButton>
#include <QProcess>
#include <QBoxLayout>
#include <QTextBlock>

class BinaryToolsDialogPrivate
{
    friend class BinaryToolsDialog;
    BinaryToolsConfigView *configView = nullptr;
};

BinaryToolsDialog::BinaryToolsDialog(QDialog *parent)
    : QDialog(parent)
    , d (new BinaryToolsDialogPrivate)
{
    setWindowTitle(tr("Binary Tools"));

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(20, 20, 20, 20);
    vLayout->setSpacing(6);

    d->configView = new BinaryToolsConfigView;
    vLayout->addWidget(d->configView);

    QHBoxLayout *buttonLayout = new QHBoxLayout(this);
    QPushButton *pbtUse = new QPushButton(tr("Use Tool"), this);
    QPushButton *pbtSave = new QPushButton(tr("Save"), this);
    QPushButton *pbtCancel = new QPushButton(tr("Cancel"), this);
    pbtUse->setDefault(true);
    auto buttonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    buttonLayout->addItem(buttonSpacer);
    buttonLayout->addWidget(pbtUse);
    buttonLayout->addWidget(pbtSave);
    buttonLayout->addWidget(pbtCancel);
    vLayout->addLayout(buttonLayout);

    connect(pbtUse, &QPushButton::clicked, [=](){
        QtConcurrent::run([=](){
            useClicked();
        });
    });
    connect(d->configView, &BinaryToolsConfigView::useCombinationCommand, [=](){
        QtConcurrent::run([=](){
            useClicked();
        });
    });
    connect(pbtSave, &QPushButton::clicked, this, &BinaryToolsDialog::saveClicked);
    connect(pbtCancel, &QPushButton::clicked, this, &BinaryToolsDialog::reject);
}

BinaryToolsDialog::~BinaryToolsDialog()
{
    if (d)
        delete d;
}

void BinaryToolsDialog::printOutput(const QString &content, OutputPane::OutputFormat format)
{
    editor.switchContext(tr("&Application Output"));
    auto outputPane = OutputPane::instance();
    QString outputContent = content;
    if (format == OutputPane::OutputFormat::NormalMessage) {
        QTextDocument *doc = outputPane->document();
        QTextBlock tb = doc->lastBlock();
        QString lastLineText = tb.text();
        QString prefix = "\n";
        if (lastLineText.isEmpty()) {
            prefix = "";
        }
        QDateTime curDatetime = QDateTime::currentDateTime();
        QString time = curDatetime.toString("hh:mm:ss");
        outputContent = prefix + time + ":" + content;
    }
    outputContent += "\n";
    OutputPane::AppendMode mode = OutputPane::AppendMode::Normal;
    outputPane->appendText(outputContent, format, mode);
}

void BinaryToolsDialog::saveClicked()
{
    d->configView->saveConfig();
}

void BinaryToolsDialog::useClicked()
{
    d->configView->saveConfig();

    QProcess proc;
    QString retMsg = tr("Error: execute command error! The reason is unknown.\n");;
    connect(&proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
        if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
            retMsg = tr("The process \"%1\" exited normally.\n").arg(proc.program());
        } else if (exitStatus == QProcess::NormalExit) {
            retMsg = tr("The process \"%1\" exited with code %2.\n")
                    .arg(proc.program(), QString::number(exitcode));
        } else {
            retMsg = tr("The process \"%1\" crashed.\n").arg(proc.program());
        }
    });

    connect(&proc, &QProcess::readyReadStandardError, [&]() {
        proc.setReadChannel(QProcess::StandardError);
        while (proc.canReadLine()) {
            QString line = QString::fromUtf8(proc.readLine());
            qInfo() << line;
            outputMsg(line, OutputPane::OutputFormat::StdErr);
        }
    });

    connect(&proc, &QProcess::readyReadStandardOutput, [&]() {
        proc.setReadChannel(QProcess::StandardOutput);
        while (proc.canReadLine()) {
            QString line = QString::fromUtf8(proc.readLine());
            qInfo() << line;
            outputMsg(line, OutputPane::OutputFormat::StdOut);
        }
    });

    QList<QString> programList = d->configView->getProgramList();
    QList<QStringList> argsList = d->configView->getArgumentsList();
    QList<QString> workingDirList = d->configView->getWorkingDirList();
    QList<QMap<QString, QVariant>> envList = d->configView->getEnvironmentList();

    for (int i =0; i < programList.size(); i++) {
        proc.setProgram(programList.at(i));
        if (!argsList.at(i).at(0).isEmpty()) {
            proc.setArguments(argsList.at(i));
        } else {
            proc.setArguments({});
        }
        proc.setWorkingDirectory(workingDirList.at(i));
        QProcessEnvironment env;
        auto iterator = envList.at(i).begin();
        while (iterator != envList.at(i).end()) {
            env.insert(iterator.key(), iterator.value().toString());
            ++iterator;
        }
        proc.setProcessEnvironment(env);
        QString startMsg = tr("Start execute command: \"%1\" \"%2\" in workspace \"%3\".\n")
                .arg(programList.at(i), argsList.at(i).join(" "), workingDirList.at(i));
        outputMsg(startMsg, OutputPane::OutputFormat::NormalMessage);
        proc.start();
        proc.waitForFinished(-1);

        outputMsg(retMsg, OutputPane::OutputFormat::NormalMessage);
        QString endMsg = tr("Execute command finished.\n");
        outputMsg(endMsg, OutputPane::OutputFormat::NormalMessage);
    }
    QDialog::reject();
}

void BinaryToolsDialog::outputMsg(const QString &content, OutputPane::OutputFormat format)
{
    QMetaObject::invokeMethod(this, "printOutput", Q_ARG(QString, content), Q_ARG(OutputPane::OutputFormat, format));
}
