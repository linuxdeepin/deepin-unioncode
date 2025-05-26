// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractaction.h"

#include <QDebug>
#include <QAction>

class AbstractActionPrivate
{
    friend class AbstractAction;
    QAction *action = nullptr;

    bool hasShortCut { false };
    QString id { "" };
    QString description { "" };
    QKeySequence keySequence {};
};

AbstractAction::AbstractAction(QAction *qAction, QObject *parent)
    : QObject(parent),
      d(new AbstractActionPrivate())
{
    Q_ASSERT(qAction);

    if (!qAction->parent())
        qAction->setParent(this);

    d->action = qAction;
    d->action->setObjectName("AbstractAction");
}

AbstractAction::~AbstractAction()
{
    delete d;
}

QAction *AbstractAction::qAction()
{
    return d->action;
}

void AbstractAction::setShortCutInfo(const QString &id, const QString &description, const QKeySequence defaultShortCut)
{
    d->hasShortCut = true;
    d->id = id;
    d->description = description;
    if(!defaultShortCut.isEmpty() && d->action->shortcut().isEmpty())
        d->action->setShortcut(defaultShortCut);
    d->keySequence = defaultShortCut.isEmpty() ? d->action->shortcut() : defaultShortCut;
}

bool AbstractAction::hasShortCut()
{
    return d->hasShortCut;
}

QString AbstractAction::id()
{
    return d->id;
}

QString AbstractAction::description()
{
    return d->description;
}

QKeySequence AbstractAction::keySequence()
{
    return d->keySequence;
}
