// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENVIRONMENTWIDGET_H
#define ENVIRONMENTWIDGET_H

#include <QTableView>
#include <common/widget/pagewidget.h>

class EnvironmentWidgetPrivate;
class EnvironmentWidget : public PageWidget
{
    Q_OBJECT

public:
    explicit EnvironmentWidget(QWidget *parent = nullptr);
    virtual ~EnvironmentWidget();

private:
    EnvironmentWidgetPrivate *const d;
};

#endif // ENVIRONMENTWIDGET_H
