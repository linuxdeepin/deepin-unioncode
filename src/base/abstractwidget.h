// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTWIDGET_H
#define ABSTRACTWIDGET_H

class AbstractWidgetPrivate;
class AbstractWidget
{
    AbstractWidgetPrivate *const d;
public:
    AbstractWidget(void *qWidget);
    virtual ~AbstractWidget();
    void* qWidget();
};

#endif // ABSTRACTWIDGET_H
