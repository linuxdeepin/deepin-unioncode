// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPILEROPTIONWIDGET_H
#define COMPILEROPTIONWIDGET_H

#include "baseoption.h"
#include "baseoptionwidgethelper.h"

class CompilerOptionWidgetPrivate;
class CompilerOptionWidget : public BaseOptionWidgetHelper, public BaseOption
{
    Q_OBJECT
public:
    explicit CompilerOptionWidget(QWidget *parent = nullptr);
    ~CompilerOptionWidget();

    QPair<QString, QList<Option>> getOptions() const override;
    QString titleName() const override;
    QString configName() const override;
    void setConfig(const QVariantMap &config) override;
    QVariantMap getConfig() const override;

private:
    CompilerOptionWidgetPrivate *const d;
};

#endif   // COMPILEROPTIONWIDGET_H
