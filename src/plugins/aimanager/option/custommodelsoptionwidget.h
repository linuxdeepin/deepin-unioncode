// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMMODELSOPTIONWIDGET
#define CUSTOMMODELSOPTIONWIDGET

#include "common/common.h"

class CustomModelsOptionWidgetPrivate;
class CustomModelsOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit CustomModelsOptionWidget(QWidget *parent = nullptr);
    ~CustomModelsOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

private:
    CustomModelsOptionWidgetPrivate *const d;
};

#endif // CUSTOMMODELSOPTIONWIDGET
