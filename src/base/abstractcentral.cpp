/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
