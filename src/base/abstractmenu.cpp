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
#include "abstractmenu.h"

#include <QMenu>
#include <QDebug>

class AbstractMenuPrivate
{
    friend class AbstractMenu;
    QMenu *menu;
};

AbstractMenu::AbstractMenu(void *qMenu)
    : d(new AbstractMenuPrivate())
{
    if (!qMenu) {
        qCritical() << "Failed, use QAction(0x0) to AbstractAction";
        abort();
    }

    d->menu = (QMenu*)(qMenu);
    QMenu::connect(d->menu, &QMenu::destroyed,
                   d->menu, [this]() {
        delete this;
    },Qt::UniqueConnection);
}

AbstractMenu::~AbstractMenu()
{
    if (d) {
        delete d;
    }
}

void *AbstractMenu::qMenu()
{
    return d->menu;
}
