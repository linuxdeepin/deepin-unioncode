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
    if (!qAction) {
        qCritical() << "Failed, use QAction(0x0) to AbstractAction";
        abort();
    }

    d->action = (QAction*)qAction;
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
    return(void*)(d->action);
}

