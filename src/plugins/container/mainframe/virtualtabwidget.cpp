// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualtabwidget.h"

VirtualTabWidget::VirtualTabWidget() :
    tabWidget(new QTabWidget())
{
    tabWidget->setTabPosition(QTabWidget::South);
    tabWidget->setTabsClosable(true);
}

VirtualTabWidget::~VirtualTabWidget()
{
    delete tabWidget;
}

QTabWidget *VirtualTabWidget::getTabWidget()
{
    return  tabWidget;
}
