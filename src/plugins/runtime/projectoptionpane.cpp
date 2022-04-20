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
#include "projectoptionpane.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>

ProjectOptionPane::ProjectOptionPane(QWidget *parent) : QGroupBox(parent)
{
    setupUI();
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

void ProjectOptionPane::runBtnClicked()
{
    emit activeRunCfgPane();
}

void ProjectOptionPane::buildBtnClicked()
{
    emit activeBuildCfgPane();
}
