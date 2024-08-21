// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rrdialog.h"
#include "debugmanager.h"
#include "services/project/projectservice.h"
#include "services/language/languageservice.h"
#include "services/window/windowelement.h"
#include "debuggerglobals.h"

#include <DDialog>
#include <DLabel>
#include <DPushButton>
#include <DSuggestButton>
#include <DFileDialog>

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;
RecordDialog::RecordDialog(QWidget *parent) :
    DDialog(parent)
{
    setupUi();
}

RecordDialog::~RecordDialog()
{
}

void RecordDialog::on_pbtnOK_clicked()
{
    auto prjService = dpfGetService(ProjectService);
    auto langService = dpfGetService(LanguageService);
    auto activePrjInfo = prjService->getActiveProjectInfo();
    auto generator = langService->create<LanguageGenerator>(activePrjInfo.kitName());
    RunCommandInfo args = generator->getRunArguments(activePrjInfo, "");

    emit startRecord(args);
    accept();
}

void RecordDialog::on_pbtnCancel_clicked()
{
    reject();
}

void RecordDialog::setupUi()
{
    setWindowTitle(tr("Record"));
    resize(400, 196);
    DWidget *mainFrame = new DWidget(this);
    auto verticalLayout = new QVBoxLayout(mainFrame);
    addContent(mainFrame);
    verticalLayout->setSpacing(10);
    verticalLayout->setContentsMargins(11, 11, 11, 11);

    auto contentLayout = new QFormLayout;
    QLabel *projectLb = new QLabel(tr("Current Project:"), this);
    QComboBox *projectCb = new QComboBox(this);
    auto prjService = dpfGetService(ProjectService);
    ProjectInfo activePrjInfo = prjService->getActiveProjectInfo();
    projectCb->addItem(activePrjInfo.kitName());
    contentLayout->addRow(projectLb, projectCb);

    QLabel *programLb = new QLabel(tr("Target:"), this);
    QLabel *programCb = new QLabel(activePrjInfo.currentProgram(), this);
    contentLayout->addRow(programLb, programCb);

    verticalLayout->addLayout(contentLayout);

    // init button
    auto horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);
    
    auto pbtnCancel = new DPushButton(this);
    pbtnCancel->setText("Cancel");
    pbtnCancel->setObjectName(QStringLiteral("pbtnCancel"));
    horizontalLayout->addWidget(pbtnCancel);

    auto pbtnOK = new DSuggestButton(this);
    pbtnOK->setText(tr("Start Record"));
    pbtnOK->setObjectName(QStringLiteral("pbtnOK"));
    horizontalLayout->addWidget(pbtnOK);

    if (activePrjInfo.language() != MWMFA_CXX)
        pbtnOK->setEnabled(false);

    QLabel *message = new QLabel(tr("Only support C/C++, and some architectures of CPU may experience anomalies."), this);
    message->setWordWrap(true);
    QPalette palette = message->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    message->setPalette(palette);

    verticalLayout->addWidget(message);
    verticalLayout->addLayout(horizontalLayout);

    QMetaObject::connectSlotsByName(this);
}

ReplayDialog::ReplayDialog(QWidget *parent) :
    DDialog(parent)
{
    setupUi();
}

ReplayDialog::~ReplayDialog()
{
}

void ReplayDialog::on_pbtnOK_clicked()
{
    emit startReplay(traceDir->text());
    accept();
}

void ReplayDialog::on_pbtnCancel_clicked()
{
    reject();
}

void ReplayDialog::setupUi()
{
    setWindowTitle(tr("Replay"));
    resize(400, 196);
    DWidget *mainFrame = new DWidget(this);
    auto verticalLayout = new QVBoxLayout(mainFrame);
    addContent(mainFrame);
    verticalLayout->setSpacing(10);
    verticalLayout->setContentsMargins(11, 11, 11, 11);

    QString defaultTraceDir = QDir::homePath() + QDir::separator() + ".local/share/rr/latest-trace";

    auto contentLayout = new QHBoxLayout;
    auto *traceLb = new QLabel(tr("Trace Directory:"), this);
    traceDir = new DLineEdit(this);
    traceDir->setText(defaultTraceDir);
    DSuggestButton *btnBrowser = new DSuggestButton(this);
    btnBrowser->setIcon(DStyle::standardIcon(style(), DStyle::SP_SelectElement));
    btnBrowser->setIconSize(QSize(24, 24));
    btnBrowser->setFixedSize(36, 36);
    connect(btnBrowser, &DSuggestButton::clicked, this, &ReplayDialog::showFileDialog);

    contentLayout->addWidget(traceLb);
    contentLayout->addWidget(traceDir);
    contentLayout->addWidget(btnBrowser);
    verticalLayout->addLayout(contentLayout);

    // init button
    auto horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);

    auto pbtnCancel = new DPushButton(this);
    pbtnCancel->setText("Cancel");
    pbtnCancel->setObjectName(QStringLiteral("pbtnCancel"));
    horizontalLayout->addWidget(pbtnCancel);

    auto pbtnOK = new DSuggestButton(this);
    pbtnOK->setText(tr("Start Replay"));
    pbtnOK->setObjectName(QStringLiteral("pbtnOK"));
    horizontalLayout->addWidget(pbtnOK);

    verticalLayout->addLayout(horizontalLayout);

    QMetaObject::connectSlotsByName(this);
}

void ReplayDialog::showFileDialog()
{
    QString dir = DFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    traceDir->text(),
                                                    DFileDialog::ShowDirsOnly
                                                            | DFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
        traceDir->setText(dir);
}
