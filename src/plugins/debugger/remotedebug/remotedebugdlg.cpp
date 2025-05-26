// SPDX-FileCopyrightText: 2022 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "remotedebugdlg.h"
#include "debugmanager.h"

#include <DDialog>
#include <DLabel>
#include <DPushButton>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
RemoteDebugDlg::RemoteDebugDlg(QWidget *parent) :
    DDialog(parent)
{
    setupUi();
}

RemoteDebugDlg::~RemoteDebugDlg()
{
}

void RemoteDebugDlg::on_pbtnOK_clicked()
{
    RemoteInfo info;
    info.executablePath = leDebugee->text();
    info.ip = leIP->text();
    info.port = lePort->text().toInt();
    info.projectPath = leProjectPath->text();
    debugManager->remoteDebug(info);
    
    accept();
}

void RemoteDebugDlg::on_pbtnCancel_clicked()
{
    reject();
}

void RemoteDebugDlg::setupUi()
{
    setWindowTitle(tr("Remote Debug"));
    resize(400, 196);
    auto verticalLayout = static_cast<QVBoxLayout *>(this->layout());
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    auto gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    auto lbDebuggee = new DLabel(this);
    lbDebuggee->setText(tr("Debuggee:"));

    gridLayout->addWidget(lbDebuggee, 3, 0, 1, 1);

    lePort = new DLineEdit(this);
    lePort->setPlaceholderText("default: 4711");
    gridLayout->addWidget(lePort, 2, 1, 1, 1);

    leIP = new DLineEdit(this);
    gridLayout->addWidget(leIP, 1, 1, 1, 1);

    auto lbPort = new DLabel(this);
    lbPort->setText(tr("Port:"));

    gridLayout->addWidget(lbPort, 2, 0, 1, 1);

    leDebugee = new DLineEdit(this);
    gridLayout->addWidget(leDebugee, 3, 1, 1, 1);

    auto lbIP = new DLabel(this);
    lbIP->setText(tr("IP:"));

    gridLayout->addWidget(lbIP, 1, 0, 1, 1);

    leParameters = new DLineEdit(this);

    gridLayout->addWidget(leParameters, 4, 1, 1, 1);

    auto lbParameters = new DLabel(this);
    lbParameters->setText(tr("Parameters:"));

    gridLayout->addWidget(lbParameters, 4, 0, 1, 1);

    leProjectPath = new DLineEdit(this);
    gridLayout->addWidget(leProjectPath, 5, 1, 1, 1);

    auto lbProjectPath = new DLabel(this);
    lbProjectPath->setText(tr("remote project root path:"));

    gridLayout->addWidget(lbProjectPath, 5, 0, 1, 1);

    verticalLayout->addLayout(gridLayout);

    auto horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);
    auto horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer);

    auto pbtnOK = new DPushButton(this);
    pbtnOK->setText(tr("OK"));
    pbtnOK->setObjectName(QStringLiteral("pbtnOK"));

    horizontalLayout->addWidget(pbtnOK);

    auto pbtnCancel = new DPushButton(this);
    pbtnCancel->setText("Cancel");
    pbtnCancel->setObjectName(QStringLiteral("pbtnCancel"));

    horizontalLayout->addWidget(pbtnCancel);

    verticalLayout->addLayout(horizontalLayout);

    QMetaObject::connectSlotsByName(this);
}
