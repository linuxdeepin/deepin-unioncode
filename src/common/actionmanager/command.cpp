// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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



