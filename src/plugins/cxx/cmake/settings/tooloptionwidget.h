// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLOPTIONWIDGET_H
#define TOOLOPTIONWIDGET_H

#include "baseoption.h"
#include "baseoptionwidgethelper.h"

class ToolOptionWidgetPrivate;
class ToolOptionWidget : public BaseOptionWidgetHelper, public BaseOption
{
    Q_OBJECT
public:
    explicit ToolOptionWidget(QWidget *parent = nullptr);
    ~ToolOptionWidget();

    QPair<QString, QList<Option>> getOptions() const override;
    QString titleName() const override;
    QString configName() const override;
    void setConfig(const QVariantMap &config) override;
    QVariantMap getConfig() const override;

private:
    ToolOptionWidgetPrivate *const d;
};

#endif   // TOOLOPTIONWIDGET_H
