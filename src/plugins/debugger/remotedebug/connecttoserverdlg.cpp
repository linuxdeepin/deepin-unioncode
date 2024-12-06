// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "connecttoserverdlg.h"
#include "debugmanager.h"

#include <DDialog>
#include <DLabel>
#include <DPushButton>
#include <DFileDialog>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
ConnectToServerDlg::ConnectToServerDlg(QWidget *parent)
    : DDialog(parent)
{
    setupUi();

    connect(selDebugEE, &DSuggestButton::clicked, this, [=]() { setFileByFileDialog(leDebugee); });
    connect(selProject, &DSuggestButton::clicked, this, [=]() { setDirByFileDialog(leProjectPath); });
    connect(selDebugInfo, &DSuggestButton::clicked, this, [=]() { setDirByFileDialog(leDebugInfo); });
}

void ConnectToServerDlg::on_pbtnOK_clicked()
{
    GdbserverInfo info;

    info.executablePath = leDebugee->text();
    info.initCommands = leCommands->toPlainText().split('\n');
    info.ip = leIP->text();
    bool ok;
    info.port = lePort->text().toInt(&ok);
    info.projectPath = leProjectPath->text();
    info.arg = leParam->text();
    info.debugInfo = leDebugInfo->text();
    if (!ok) {
        lePort->clear();
        lePort->setPlaceholderText(tr("Port must be number"));
        return;
    }

    debugManager->connectToGdbServer(info);
    accept();
}

void ConnectToServerDlg::on_pbtnCancel_clicked()
{
    reject();
}

void ConnectToServerDlg::setupUi()
{
    setWindowTitle(tr("Connect To GdbServer"));
    setIcon(QIcon::fromTheme("ide"));
    auto verticalLayout = static_cast<QVBoxLayout *>(this->layout());
    verticalLayout->setContentsMargins(20, 11, 20, 11);
    auto gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(0, 0, 10, 0);

    auto lbIP = new DLabel(this);
    lbIP->setText(tr("IP:"));
    leIP = new DLineEdit(this);
    gridLayout->addWidget(lbIP, 1, 0);
    gridLayout->addWidget(leIP, 1, 1, 1, 2);

    lePort = new DLineEdit(this);
    auto lbPort = new DLabel(this);
    lbPort->setText(tr("Port:"));
    gridLayout->addWidget(lbPort, 2, 0);
    gridLayout->addWidget(lePort, 2, 1, 1, 2);

    auto lbDebuggee = new DLabel(this);
    lbDebuggee->setText(tr("Debuggee:"));
    leDebugee = new DLineEdit(this);
    selDebugEE = new DSuggestButton(this);
    selDebugEE->setIcon(DStyle::standardIcon(style(), DStyle::SP_SelectElement));
    gridLayout->addWidget(lbDebuggee, 3, 0);
    gridLayout->addWidget(leDebugee, 3, 1);
    gridLayout->addWidget(selDebugEE, 3, 2);

    auto lbParam = new DLabel(this);
    lbParam->setText(tr("Param:"));
    leParam = new DLineEdit(this);
    gridLayout->addWidget(lbParam, 4, 0);
    gridLayout->addWidget(leParam, 4, 1, 1, 2);

    leCommands = new DTextEdit(this);
    auto lbCommands = new DLabel(this);
    lbCommands->setText(tr("Init Commands:"));
    gridLayout->addWidget(lbCommands, 5, 0);
    gridLayout->addWidget(leCommands, 5, 1, 1, 2);

    leProjectPath = new DLineEdit(this);
    auto lbProjectPath = new DLabel(this);
    lbProjectPath->setText(tr("Local Project Path:"));
    selProject = new DSuggestButton(this);
    selProject->setIcon(DStyle::standardIcon(style(), DStyle::SP_SelectElement));
    gridLayout->addWidget(lbProjectPath, 6, 0);
    gridLayout->addWidget(leProjectPath, 6, 1);
    gridLayout->addWidget(selProject, 6, 2);

    leDebugInfo = new DLineEdit(this);
    auto lbDebugInfo = new DLabel(this);
    lbDebugInfo->setText(tr("Debug Info:"));
    selDebugInfo = new DSuggestButton(this);
    selDebugInfo->setIcon(DStyle::standardIcon(style(), DStyle::SP_SelectElement));
    gridLayout->addWidget(lbDebugInfo, 7, 0);
    gridLayout->addWidget(leDebugInfo, 7, 1);
    gridLayout->addWidget(selDebugInfo, 7, 2);

    verticalLayout->addLayout(gridLayout);

    auto horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);
    auto pbtnCancel = new DPushButton(this);
    pbtnCancel->setFixedWidth(173);
    pbtnCancel->setText("Cancel");
    pbtnCancel->setObjectName(QStringLiteral("pbtnCancel"));
    horizontalLayout->addWidget(pbtnCancel);

    DVerticalLine *vLine = new DVerticalLine;
    vLine->setObjectName("VLine");
    vLine->setFixedHeight(30);
    horizontalLayout->addWidget(vLine);

    auto pbtnOK = new DSuggestButton(this);
    pbtnOK->setFixedWidth(173);
    pbtnOK->setText(tr("OK"));
    pbtnOK->setObjectName(QStringLiteral("pbtnOK"));
    horizontalLayout->addWidget(pbtnOK);

    verticalLayout->addLayout(horizontalLayout);
    QMetaObject::connectSlotsByName(this);
}

void ConnectToServerDlg::setParam(const GdbserverInfo &info)
{
    leIP->setText(info.ip);
    lePort->setText(QString::number(info.port));
    leDebugee->setText(info.executablePath);
    leParam->setText(info.arg);
    leCommands->setText(info.initCommands.join('\n'));
    leProjectPath->setText(info.projectPath);
    leDebugInfo->setText(info.debugInfo);
}

void ConnectToServerDlg::setDirByFileDialog(DLineEdit *edit)
{
    if (!edit)
        return;

    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = DFileDialog::getExistingDirectory(this, "", dir);
    if (filePath.isEmpty() && !QFileInfo(filePath).exists())
        return;

    edit->setText(filePath);
}

void ConnectToServerDlg::setFileByFileDialog(DLineEdit *edit)
{
    if (!edit)
        return;

    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString filePath = DFileDialog::getOpenFileName(this, "", dir);
    if (filePath.isEmpty() && !QFileInfo(filePath).exists())
        return;

    edit->setText(filePath);
}
