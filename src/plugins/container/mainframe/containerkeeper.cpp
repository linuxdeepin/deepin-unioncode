// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "containerkeeper.h"

#include <QDebug>
#include <QTabWidget>

ContainerKeeper::ContainerKeeper(QObject *parent)
    : QObject (parent)
{

}

ContainerKeeper::~ContainerKeeper()
{

}

ContainerKeeper *ContainerKeeper::instance()
{
    static ContainerKeeper ins;
    return &ins;
}

QTabWidget *ContainerKeeper::tabWidget(){
    if (!qTabWidget) {
        auto virtualTabWidget = new VirtualTabWidget;
        qTabWidget = virtualTabWidget->getTabWidget();
    }
    return qTabWidget;
}
