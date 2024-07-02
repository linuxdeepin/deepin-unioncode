// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "abstractwidget.h"

#include <DWidget>

#include <QDebug>
#include <QApplication>

DWIDGET_USE_NAMESPACE

class AbstractWidgetPrivate
{
    friend class AbstractWidget;
    DWidget *qWidget;
};

AbstractWidget::AbstractWidget(void *qWidget)
    : d(new AbstractWidgetPrivate)
{
    Q_ASSERT(qWidget);

    d->qWidget = static_cast<DWidget*>(qWidget);
    QObject::connect(d->qWidget, &DWidget::destroyed,
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
