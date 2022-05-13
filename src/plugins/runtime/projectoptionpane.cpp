/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#include "projectoptionpane.h"
#include "services/project/projectservice.h"
#include "kitsmanagerwidget.h"
#include "environmentmanagerwidget.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>

#define SAFE_DELETE(pData) { try { delete pData; } catch (...) { Q_ASSERT(false); } pData = nullptr; }

using namespace dpfservice;
ProjectOptionPane::ProjectOptionPane(QWidget *parent) : QGroupBox(parent)
{
    setupUI();
}

ProjectOptionPane::~ProjectOptionPane()
{
    SAFE_DELETE(kitManagerWidget)
    SAFE_DELETE(envManagerWidget)
}

void ProjectOptionPane::setupUI()
{
    auto titleFont = [](QWidget *w) {
        if (w) {
            QFont ft;
            ft.setBold(true);
            ft.setPointSize(12);
            w->setFont(ft);
        }
    };

    this->setTitle(tr("Project Options"));
    auto leftLayout = new QVBoxLayout();
    setLayout(leftLayout);

    auto kitButton = new QPushButton(tr("Manage Kits..."), this);
    leftLayout->addWidget(kitButton);
    connect(kitButton, &QPushButton::clicked, this, &ProjectOptionPane::showKitDialog);
    initializeKitManageWidget();

    // Active Project ui.
    auto activeProjectLabel = new QLabel(tr("Active Project"), this);
    titleFont(activeProjectLabel);
    auto activeProjectCombo = new QComboBox();
    activeProjectCombo->addItem("unioncode");
    leftLayout->addWidget(activeProjectLabel);
    leftLayout->addWidget(activeProjectCombo);

    // Build and Run ui.
    auto buildRunLabel = new QLabel(tr("Build & Run"), this);
    titleFont(buildRunLabel);
    buildButton = new QPushButton(tr("Build"), this);
    runButton = new QPushButton(tr("Run"), this);
    connect(buildButton, &QPushButton::clicked, this, &ProjectOptionPane::buildBtnClicked);
    connect(runButton, &QPushButton::clicked, this, &ProjectOptionPane::runBtnClicked);

    leftLayout->addWidget(buildRunLabel);
    leftLayout->addWidget(buildButton);
    leftLayout->addWidget(runButton);
    leftLayout->addStretch();
}

void ProjectOptionPane::initializeKitManageWidget()
{
    // Initialize kit config panel.
    auto &ctx = dpfInstance.serviceContext();
    projectService = ctx.service<ProjectService>(ProjectService::name());

    if (projectService) {
        kitManagerWidget = new KitsManagerWidget();
        projectService->insertOptionPanel("kit", kitManagerWidget);
        envManagerWidget = new EnvironmentManagerWidget();
        projectService->insertOptionPanel("Environment", envManagerWidget);
    }
}

void ProjectOptionPane::runBtnClicked()
{
    emit activeRunCfgPane();
}

void ProjectOptionPane::showKitDialog()
{
    projectService->showProjectOptionsDlg("kit", "");
}

void ProjectOptionPane::buildBtnClicked()
{
    emit activeBuildCfgPane();
}
