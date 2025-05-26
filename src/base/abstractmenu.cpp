// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractmenu.h"

#include <QDebug>

class AbstractMenuPrivate
{
    friend class AbstractMenu;
    DMenu *menu;
    QList<AbstractAction *> actionlist;
};

AbstractMenu::AbstractMenu(DMenu *qMenu)
    : d(new AbstractMenuPrivate())
{
    Q_ASSERT(qMenu);

    d->menu = qMenu;
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

DMenu *AbstractMenu::qMenu()
{
    return d->menu;
}

void AbstractMenu::addAction(AbstractAction *action)
{
    d->actionlist.append(action);
    d->menu->addAction(action->qAction());
}

QList<AbstractAction *> AbstractMenu::actionList()
{
    return d->actionlist;
}
