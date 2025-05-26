// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JAVAOPTIONWIDGET_H
#define JAVAOPTIONWIDGET_H

#include "common/common.h"

class JavaOptionWidgetPrivate;
class JavaOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit JavaOptionWidget(QWidget *parent = nullptr);
    ~JavaOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

signals:

public slots:
private:
    JavaOptionWidgetPrivate *const d;
};

#endif // JAVAOPTIONWIDGET_H
