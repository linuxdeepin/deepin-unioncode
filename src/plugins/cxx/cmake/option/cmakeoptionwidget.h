// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEOPTIONWIDGET_H
#define CMAKEOPTIONWIDGET_H

#include "common/common.h"

class CMakeOptionWidgetPrivate;
class CMakeOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit CMakeOptionWidget(QWidget *parent = nullptr);
    ~CMakeOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

signals:

public slots:
private:
    CMakeOptionWidgetPrivate *const d;
};

#endif // CMAKEOPTIONWIDGET_H
