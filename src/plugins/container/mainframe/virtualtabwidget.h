// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALTABWIDGET_H
#define VIRTUALTABWIDGET_H

#include <QTabWidget>

class VirtualTabWidget : QTabWidget
{
    Q_OBJECT
public:
    VirtualTabWidget();
    virtual ~VirtualTabWidget();

    QTabWidget *getTabWidget();
private:
    QTabWidget *tabWidget;
};

#endif // VIRTUALTABWIDGET_H
