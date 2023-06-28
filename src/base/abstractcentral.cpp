// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractcentral.h"

#include <QDebug>
#include <QWidget>

class AbstractCentralPrivate
{
    friend class AbstractCentral;
    QWidget *widget;
};

AbstractCentral::AbstractCentral(void *qwidget)
    : d(new AbstractCentralPrivate())
{
    if (!qwidget) {
        qCritical() << "Failed, use QWidget(0x0) to AbstractCentral";
        abort();
    }

    d->widget = (QWidget*)qwidget;
    QWidget::connect(d->widget, &QWidget::destroyed,
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
