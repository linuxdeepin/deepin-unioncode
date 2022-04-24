/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "runconfigpane.h"
#include "services/project/projectservice.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QComboBox>
#include <QFormLayout>

using namespace dpfservice;
RunConfigPane::RunConfigPane(QWidget *parent) : QWidget(parent)
{
    setupUi();

    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projService = ctx.service<ProjectService>(ProjectService::name());
    if (projService) {
        connect(projService, &ProjectService::projectConfigureDone, [this](){
            updateUi();
        });
    }
}

void RunConfigPane::showFileDialog()
{
    QString outputDirectory = QFileDialog::getExistingDirectory(this, tr("Working directory:"));
    if (!outputDirectory.isEmpty()) {
        workingDirLineEdit->setText(outputDirectory.toUtf8());
    }
}

void RunConfigPane::setupUi()
{
    if (!vLayout) {
        vLayout = new QVBoxLayout(this);
    }

    // Head titile.
    QLabel *runLabel = new QLabel(this);
    runLabel->setText(tr("Run"));
    QFont ft;
    ft.setBold(true);
    ft.setPointSize(16);
    runLabel->setFont(ft);

    // run config ui.
    QHBoxLayout *runCfgLayout = new QHBoxLayout(this);
    QLabel *runCfgLabel = new QLabel(this);
    runCfgLabel->setText(tr("Run config:"));
    QComboBox *projList = new QComboBox(this);
    projList->addItem("project1");
    projList->addItem("project2");
    runCfgLayout->addWidget(runCfgLabel);
    runCfgLayout->addWidget(projList);
    projList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QFrame *mainFrame = new QFrame(this);
    mainFrame->setObjectName("mainframe");
    QFormLayout *formLayout = new QFormLayout(mainFrame);

    // excutable label ui.
    QLabel *excutableLabel = new QLabel(mainFrame);
    excutableLabel->setText(tr("Here is the executable path"));
    formLayout->addRow(tr("Excutable:"), excutableLabel);

    // command line ui.
    QLineEdit *cmdArgsLineEdit = new QLineEdit(mainFrame);
    formLayout->addRow(tr("Command line arguments:"), cmdArgsLineEdit);

    // working directory ui.
    QHBoxLayout *browLayout = new QHBoxLayout(mainFrame);
    auto button = new QPushButton(mainFrame);
    button->setText(tr("Browse..."));
    workingDirLineEdit = new QLineEdit(mainFrame);
    browLayout->addWidget(workingDirLineEdit);
    browLayout->addWidget(button);
    formLayout->addRow(tr("Working directory:"), browLayout);
    connect(button, SIGNAL(clicked()), this, SLOT(showFileDialog()));

    // insert to layout.
    mainFrame->setLayout(formLayout);

    vLayout->addWidget(runLabel);
    vLayout->addLayout(runCfgLayout);
    vLayout->addWidget(mainFrame);

    vLayout->setMargin(0);
}

void RunConfigPane::updateUi()
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projService = ctx.service<ProjectService>(ProjectService::name());
    if (projService) {
        if (projService->getDefaultOutputPath) {
            QString workingDir = projService->getDefaultOutputPath();
            workingDirLineEdit->setText(workingDir);
        }
    }
}
