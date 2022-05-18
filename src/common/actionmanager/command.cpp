/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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

#include "command.h"

#include <QAction>

class ActionPrivate
{
public:
    ActionPrivate();
    virtual ~ActionPrivate();

private:
    QString id;
    QKeySequence shortcutKey;
    QString description;
    QAction *action;

    friend class Action;
};

ActionPrivate::ActionPrivate()
    : action(nullptr)
{

}

ActionPrivate::~ActionPrivate()
{

}

Action::Action(QString id, QAction *action)
    : d(new ActionPrivate())
{
    d->id = id;
    d->action = action;
}

Action::~Action()
{
    if (d) {
        delete d;
    }
}

QString Action::id() const
{
    return d->id;
}

QAction *Action::action() const
{
    return d->action;
}

void Action::setKeySequence(const QKeySequence &key)
{
    d->shortcutKey = key;
    if (d->action)
        d->action->setShortcut(key);
}

QKeySequence Action::keySequence() const
{
    return d->shortcutKey;
}

void Action::setDescription(const QString &text)
{
    d->description = text;
    if (d->action)
        d->action->setText(text);
}

QString Action::description() const
{
    return d->description;
}



