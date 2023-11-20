// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractnav.h"
#include "abstractaction.h"

#include <DToolBar>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class AbstractNavPrivate
{
    friend class AbstractNav;
    DToolBar* qToolBar = nullptr;
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

    d->qToolBar = static_cast<DToolBar*>(qToolBar);
    QObject::connect(d->qToolBar, &DToolBar::destroyed,
                     d->qToolBar, [this](){
        delete this;
    }, Qt::UniqueConnection);
}
