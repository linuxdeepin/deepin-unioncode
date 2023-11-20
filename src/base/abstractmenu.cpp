// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractmenu.h"

#include <DMenu>

#include <QDebug>

DWIDGET_USE_NAMESPACE

class AbstractMenuPrivate
{
    friend class AbstractMenu;
    DMenu *menu;
};

AbstractMenu::AbstractMenu(void *qMenu)
    : d(new AbstractMenuPrivate())
{
    if (!qMenu) {
        qCritical() << "Failed, use QAction(0x0) to AbstractAction";
        abort();
    }

    d->menu = static_cast<DMenu*>(qMenu);
    DMenu::connect(d->menu, &DMenu::destroyed,
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
