// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTWIDGET_H
#define ABSTRACTWIDGET_H

#include <QIcon>

class AbstractWidgetPrivate;
class AbstractWidget
{
    AbstractWidgetPrivate *const d;
public:
    AbstractWidget(void *qWidget);
    virtual ~AbstractWidget();
    void* qWidget();
    void setDisplayIcon(QIcon icon) { displayIcon = icon; }
    QIcon getDisplayIcon() { return displayIcon; }
private:
    QIcon displayIcon;
};

#endif // ABSTRACTWIDGET_H
