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
#include "abstractconsole.h"

#include <QWidget>
#include <QDebug>

class AbstractConsolePrivate
{
    friend class AbstractConsole;
    QWidget *qWidget = nullptr;
};

AbstractConsole::AbstractConsole(void *qWidget)
    : d(new AbstractConsolePrivate)
{
    if (!qWidget) {
        qCritical() << "Failed, use QWidget(0x0) to AbstractWidget";
        abort();
    }

    qInfo() << "AbstractConsole Construct from: " << (QWidget*)(qWidget);
    d->qWidget = (QWidget*)qWidget;
    d->qWidget->connect(d->qWidget, &QWidget::destroyed,
                        d->qWidget, [this](QObject *obj){
        if (obj == d->qWidget) {
            qInfo() << "AbstractConsole QWidget::destroyed" << obj;
            delete this;
        }
    }, Qt::DirectConnection);
}

AbstractConsole::~AbstractConsole()
{
    if (d) {
        delete d;
    }
}

void *AbstractConsole::qWidget()
{
    return d->qWidget;
}

