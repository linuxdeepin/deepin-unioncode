// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractnav.h"
#include "abstractaction.h"

#include <QToolBar>
#include <QDebug>

class AbstractNavPrivate
{
    friend class AbstractNav;
    QToolBar* qToolBar = nullptr;
    std::list<AbstractAction*> actions{};
};

AbstractNav::~AbstractNav()
{
    if (d) {
        delete d;
    }
}

void *AbstractNav::qToolbar()
{
    return d->qToolBar;
}

AbstractNav::AbstractNav(void *qToolBar)
    : d(new AbstractNavPrivate())
{
    if (!qToolBar) {
        qCritical() << "Failed, use QToolbar(0x0) to AbstractMenuBar";
        abort();
    }

    d->qToolBar = static_cast<QToolBar*>(qToolBar);
    QObject::connect(d->qToolBar, &QToolBar::destroyed,
                     d->qToolBar, [this](){
        delete this;
    }, Qt::UniqueConnection);
}
