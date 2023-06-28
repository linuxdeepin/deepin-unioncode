// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractwidget.h"

#include <QWidget>
#include <QDebug>
#include <QApplication>

class AbstractWidgetPrivate
{
    friend class AbstractWidget;
    QWidget *qWidget;
};

AbstractWidget::AbstractWidget(void *qWidget)
    : d(new AbstractWidgetPrivate)
{
    if (!qWidget) {
        qCritical() << "Failed, use QWidget(0x0) to AbstractWidget";
        abort();
    }

    d->qWidget = (QWidget*)qWidget;
    QObject::connect(d->qWidget, &QWidget::destroyed,
                     d->qWidget, [this](QObject *obj){
        if (obj == d->qWidget) {
            delete this;
        }
    }, Qt::DirectConnection);
}

AbstractWidget::~AbstractWidget()
{
    if (d)
        delete d;
}

void *AbstractWidget::qWidget()
{
    return d->qWidget;
}
