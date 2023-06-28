// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERALOPTIONWIDGET_H
#define GENERALOPTIONWIDGET_H

#include "common/common.h"

class GeneralOptionWidgetPrivate;
class GeneralOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit GeneralOptionWidget(QWidget *parent = nullptr);
    ~GeneralOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

signals:

public slots:
private:
    GeneralOptionWidgetPrivate *const d;
};

#endif // GENERALOPTIONWIDGET_H
