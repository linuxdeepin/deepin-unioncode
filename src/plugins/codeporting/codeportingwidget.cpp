// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codeportingwidget.h"
#include "codeportingmanager.h"

#include <DPushButton>
#include <DButtonBox>
#include <DStackedWidget>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

CodePortingWidget::CodePortingWidget(QWidget *parent)
    : DWidget(parent)
{
    outputPane = CodePortingManager::instance()->getOutputPane();
    reportPane = CodePortingManager::instance()->getReportPane();

    initUI();
}

void CodePortingWidget::initUI()
{
    DButtonBox *btnBox = new DButtonBox(this);
    btnBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    DButtonBoxButton *outputBtn = new DButtonBoxButton(tr("C&ode Porting"));
    DButtonBoxButton *reportBtn = new DButtonBoxButton(tr("Porting &Report"));
    QList<DButtonBoxButton*> list;
    list.append(outputBtn);
    list.append(reportBtn);
    btnBox->setButtonList(list, true);

    DStackedWidget *tabWidget = new DStackedWidget();
    tabWidget->addWidget(outputPane);
    tabWidget->addWidget(reportPane);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(btnBox);
    vLayout->addWidget(tabWidget);

    auto initConn = [tabWidget, list] (DButtonBoxButton* btn) {
        connect(btn, &DButtonBoxButton::clicked, [=]{
            tabWidget->setCurrentIndex(list.indexOf(btn));
        });
    };

    initConn(outputBtn);
    initConn(reportBtn);

    outputBtn->setChecked(true);
}
