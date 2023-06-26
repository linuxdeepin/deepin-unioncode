// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    d->menuBar = static_cast<QMenuBar*>(qMenuBar);
    QMenuBar::connect(d->menuBar, &QMenuBar::destroyed,
                      d->menuBar, [this]() {
        delete this;
    }, Qt::UniqueConnection);
}
