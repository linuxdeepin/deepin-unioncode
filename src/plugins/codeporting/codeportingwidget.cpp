// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeportingwidget.h"
#include "codeportingmanager.h"

#include <DPushButton>
#include <DButtonBox>
#include <DStackedWidget>
#include <DComboBox>

#include <QAction>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

CodePortingWidget::CodePortingWidget(QWidget *parent)
    : DFrame(parent)
{
    outputPane = CodePortingManager::instance()->getOutputPane();
    reportPane = CodePortingManager::instance()->getReportPane();

    initUI();
}

void CodePortingWidget::initUI()
{
    DStyle::setFrameRadius(this, 0);
    setLineWidth(0);

    DButtonBox *btnBox = new DButtonBox(this);
    btnBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    DButtonBoxButton *outputBtn = new DButtonBoxButton(tr("C&ode Porting"));
    DButtonBoxButton *reportBtn = new DButtonBoxButton(tr("Porting &Report"));
    QList<DButtonBoxButton*> list;
    list.append(outputBtn);
    list.append(reportBtn);
    btnBox->setButtonList(list, true);

    DComboBox *reportTab = new DComboBox();
    reportTab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    reportTab->addItem(REPORT_SRCLIST);
    reportTab->addItem(REPORT_LIBLIST);
    reportTab->hide();

    connect(reportTab, &QComboBox::currentTextChanged, reportPane, &ReportPane::onChangeReportList);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(btnBox);
    hLayout->addWidget(reportTab);
    hLayout->setAlignment(btnBox, Qt::AlignLeft);
    hLayout->setAlignment(reportTab, Qt::AlignRight);

    DStackedWidget *tabWidget = new DStackedWidget();
    tabWidget->addWidget(outputPane);
    tabWidget->addWidget(reportPane);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(tabWidget);

    connect(outputBtn, &DButtonBoxButton::clicked, [=]{
        tabWidget->setCurrentIndex(list.indexOf(outputBtn));
        reportTab->hide();
    });

    connect(reportBtn, &DButtonBoxButton::clicked, [=]{
        tabWidget->setCurrentIndex(list.indexOf(reportBtn));
        reportTab->show();
    });

    outputBtn->setChecked(true);
}
