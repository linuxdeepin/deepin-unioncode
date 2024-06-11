// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KITOPTIONWIDGET_H
#define KITOPTIONWIDGET_H

#include "baseoption.h"
#include "global_define.h"

#include <QWidget>

class KitOptionWidgetPrivate;
class KitOptionWidget : public QWidget, public BaseOption
{
    Q_OBJECT
public:
    explicit KitOptionWidget(QWidget *parent = nullptr);
    ~KitOptionWidget();

    void setOptions(const QPair<QString, QList<Option>> &options);
    QString titleName() const override;
    QString configName() const override;
    void setConfig(const QVariantMap &config) override;
    QVariantMap getConfig() const override;

private:
    KitOptionWidgetPrivate *const d;
};

#endif   // KITOPTIONWIDGET_H
