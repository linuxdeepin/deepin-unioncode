// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BEHAVIORWIDGET_H
#define BEHAVIORWIDGET_H

#include "common/widget/pagewidget.h"

class BehaviorWidgetPrivate;
class BehaviorWidget : public PageWidget
{
    Q_OBJECT

public:
    explicit BehaviorWidget(QWidget *parent = nullptr);
    ~BehaviorWidget();

    void setUserConfig(const QMap<QString, QVariant> &map) override;
    void getUserConfig(QMap<QString, QVariant> &map) override;

private:
    BehaviorWidgetPrivate *const d;
};

#endif   // BEHAVIORWIDGET_H
