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
#include "overviewpane.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>

OverviewPane::OverviewPane(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void OverviewPane::showFileDialog()
{
    QString outputDirectory = QFileDialog::getExistingDirectory(this, "Output directory");
    outputDirEdit->setText(outputDirectory.toUtf8());
}

void OverviewPane::setupUi()
{
    if (!vLayout) {
        vLayout = new QVBoxLayout(this);
    }

    QLabel *overviewLabel = new QLabel(this);
    overviewLabel->setText(tr("Overview"));
    QFont ft;
    ft.setBold(true);
    ft.setPointSize(16);
    overviewLabel->setFont(ft);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    QLabel *label = new QLabel(this);
    label->setText(tr("Output direcotry:"));
    outputDirEdit = new QLineEdit(this);
    auto button = new QPushButton(this);
    button->setText(tr("Browse..."));

    hLayout->addWidget(label);
    hLayout->addWidget(outputDirEdit);
    hLayout->addWidget(button);
    hLayout->setSpacing(10);

    vLayout->setSpacing(0);
    vLayout->setMargin(0);

    connect(button, SIGNAL(clicked()), this, SLOT(showFileDialog()));

    this->setLayout(vLayout);

    vLayout->addWidget(overviewLabel);
    vLayout->addLayout(hLayout);
}
