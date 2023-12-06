// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytoolsdialog.h"
#include "binarytoolsconfigview.h"
#include "common/util/eventdefinitions.h"

#include <DPushButton>
#include <DDialogButtonBox>
#include <DFrame>
#include <DSuggestButton>
#include <DSizeMode>

#include <QIcon>
#include <QProcess>
#include <QBoxLayout>
#include <QTextBlock>
#include <DFrame>

DWIDGET_USE_NAMESPACE
class BinaryToolsDialogPrivate
{
    friend class BinaryToolsDialog;
    BinaryToolsConfigView *configView = nullptr;
    DDialogButtonBox *buttons = nullptr;
};

BinaryToolsDialog::BinaryToolsDialog(QDialog *parent)
    : DDialog(parent)
    , d (new BinaryToolsDialogPrivate)
{
    setWindowTitle(tr("Binary Tools"));
    setFixedSize(644,676);
    setIcon(QIcon::fromTheme("ide"));

    DWidget *mainFrame = new DWidget(this);
    addContent(mainFrame);
    QVBoxLayout *vLayout = new QVBoxLayout(mainFrame);
    d->configView = new BinaryToolsConfigView(mainFrame);
    vLayout->addWidget(d->configView);
    vLayout->addStretch();

    QHBoxLayout * buttonLayout = new QHBoxLayout(mainFrame);
    DPushButton *cancelButton = new DPushButton(tr("Cancel"));
    DPushButton *saveButton = new DPushButton(tr("Save Configuration"));
    DSuggestButton *applyButton = new DSuggestButton(tr("Use Tool"));
    cancelButton->setFixedWidth(173);
    saveButton->setFixedWidth(173);
    applyButton->setFixedWidth(173);

    DVerticalLine *line1 = new DVerticalLine;
    line1->setObjectName("VLine");
    line1->setFixedHeight(DSizeModeHelper::element(20, 30));

    DVerticalLine *line2 = new DVerticalLine;
    line2->setObjectName("VLine");
    line2->setFixedHeight(DSizeModeHelper::element(20, 30));

    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(line1);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(line2);
    buttonLayout->addWidget(applyButton);

    buttonLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    vLayout->addLayout(buttonLayout);

    connect(d->configView, &BinaryToolsConfigView::useCombinationCommand, [=](){
        QtConcurrent::run([=](){
            useClicked();
        });
    });

    connect(applyButton, &DSuggestButton::clicked, [=](){
        QtConcurrent::run([=](){
            useClicked();
        });
    });

     connect(saveButton, &DPushButton::clicked, this, &BinaryToolsDialog::saveClicked);

     connect(saveButton, &DPushButton::clicked, this, &BinaryToolsDialog::accept);
     connect(cancelButton, &DPushButton::clicked, this, &BinaryToolsDialog::reject);
     connect(applyButton, &DSuggestButton::clicked, this, &BinaryToolsDialog::accept);
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
}

void BinaryToolsDialog::outputMsg(const QString &content, OutputPane::OutputFormat format)
{
    QMetaObject::invokeMethod(this, "printOutput", Q_ARG(QString, content), Q_ARG(OutputPane::OutputFormat, format));
}
