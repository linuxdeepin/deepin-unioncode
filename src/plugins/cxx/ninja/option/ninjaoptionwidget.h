// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NINJAOPTIONWIDGET_H
#define NINJAOPTIONWIDGET_H

#include "common/common.h"

class NinjaOptionWidgetPrivate;
class NinjaOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit NinjaOptionWidget(QWidget *parent = nullptr);
    ~NinjaOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

signals:

public slots:
private:
    NinjaOptionWidgetPrivate *const d;
};

#endif // NINJAOPTIONWIDGET_H
