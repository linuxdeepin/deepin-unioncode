// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H

#include "qtermwidget.h" // 3drparty
#include "ColorScheme.h"

#include <QDir>

class ContainerWidget : public QTermWidget
{
    Q_OBJECT
public:
    static ContainerWidget *instance();
    explicit ContainerWidget(QWidget *parent = nullptr);
    virtual ~ContainerWidget();
};

#endif // CONTAINERWIDGET_H
