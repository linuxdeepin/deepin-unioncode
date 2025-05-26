// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADLEOPTIONWIDGET_H
#define GRADLEOPTIONWIDGET_H

#include "common/common.h"

class GradleOptionWidgetPrivate;
class GradleOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit GradleOptionWidget(QWidget *parent = nullptr);
    ~GradleOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

signals:

public slots:
private:
    GradleOptionWidgetPrivate *const d;
};

#endif // GRADLEOPTIONWIDGET_H
