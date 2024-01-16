// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractaction.h"

#include <QDebug>
#include <QAction>

class AbstractActionPrivate
{
    friend class AbstractAction;
    QAction *action = nullptr;
};

AbstractAction::AbstractAction(void *qAction)
    : d(new AbstractActionPrivate())
{
    Q_ASSERT(qAction);

    d->action = static_cast<QAction*>(qAction);
    d->action->setObjectName("AbstractAction");
    QAction::connect(d->action, &QAction::destroyed,
                     d->action, [this]() {
        delete this;
    }, Qt::UniqueConnection);
}

AbstractAction::~AbstractAction()
{
    if (d) {
        delete d;
    }
}

void *AbstractAction::qAction()
{
    return static_cast<void*>(d->action);
}

