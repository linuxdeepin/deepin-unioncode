// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGGEROPTIONWIDGET_H
#define DEBUGGEROPTIONWIDGET_H

#include "baseoption.h"
#include "baseoptionwidgethelper.h"

class DebuggerOptionWidgetPrivate;
class DebuggerOptionWidget : public BaseOptionWidgetHelper, public BaseOption
{
    Q_OBJECT
public:
    explicit DebuggerOptionWidget(QWidget *parent = nullptr);
    ~DebuggerOptionWidget();

    QPair<QString, QList<Option>> getOptions() const override;
    QString titleName() const override;
    QString configName() const override;
    void setConfig(const QVariantMap &config) override;
    QVariantMap getConfig() const override;

private:
    DebuggerOptionWidgetPrivate *const d;
};

#endif   // DEBUGGEROPTIONWIDGET_H
