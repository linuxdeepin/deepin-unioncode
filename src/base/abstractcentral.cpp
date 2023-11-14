// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractcentral.h"

#include <DWidget>

#include <QDebug>

DWIDGET_USE_NAMESPACE

class AbstractCentralPrivate
{
    friend class AbstractCentral;
    DWidget *widget;
};

AbstractCentral::AbstractCentral(void *qwidget)
    : d(new AbstractCentralPrivate())
{
    if (!qwidget) {
        qCritical() << "Failed, use QWidget(0x0) to AbstractCentral";
        abort();
    }

    d->widget = static_cast<DWidget*>(qwidget);
    DWidget::connect(d->widget, &DWidget::destroyed,
                     d->widget, [this](){
        delete this;
    }, Qt::UniqueConnection);
}

AbstractCentral::~AbstractCentral()
{
    if (d) {
        delete d;
    }
}

void *AbstractCentral::qWidget()
{
    return d->widget;
}
