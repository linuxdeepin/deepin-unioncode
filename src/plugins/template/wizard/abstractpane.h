// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTPANE_H
#define ABSTRACTPANE_H

#include <DWidget>

using DTK_WIDGET_NAMESPACE::DWidget;
class AbstractPane : public DWidget
{
public:
    explicit AbstractPane(DWidget *parent = nullptr) {}
    virtual QMap<QString, QVariant> getValue() = 0;
private:
    virtual void setupUi() = 0;
};

#endif // ABSTRACTPANE_H
