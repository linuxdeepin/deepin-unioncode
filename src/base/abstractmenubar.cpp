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
#include "abstractmenubar.h"
#include "abstractmenu.h"
#include "abstractaction.h"

#include <QMenuBar>
#include <QDebug>

class AbstractMenuBarPrivate
{
    friend class AbstractMenuBar;
    QMenuBar *menuBar;
};

AbstractMenuBar::~AbstractMenuBar()
{
    if (d) {
        delete d;
    }
}

void *AbstractMenuBar::qMenuBar()
{
    return d->menuBar;
}

AbstractMenuBar::AbstractMenuBar(void *qMenuBar)
    : d(new AbstractMenuBarPrivate())
{
    if (!qMenuBar) {
        qCritical() << "Failed, use QMenuBar(0x0) to AbstractMenuBar";
        abort();
    }

    d->menuBar = (QMenuBar*)qMenuBar;
    QMenuBar::connect(d->menuBar, &QMenuBar::destroyed,
                      d->menuBar, [this]() {
        delete this;
    }, Qt::UniqueConnection);
}
