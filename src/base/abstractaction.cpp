// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractaction.h"

#include <QDebug>
#include <QAction>

class AbstractActionPrivate {
public:
    explicit AbstractActionPrivate(QAction* qAction) : action(qAction) {}

    QAction* action = nullptr;
    bool hasShortCut = false;
    QString id = "";
    QString description = "";
    QKeySequence keySequence{};
};

AbstractAction::AbstractAction(QAction* qAction, QObject* parent)
    : QObject(parent)
    , d(new AbstractActionPrivate(qAction))
{
    Q_ASSERT(qAction);
    if (!qAction->parent()) {
        qAction->setParent(this);
    }

    d->action->setObjectName("AbstractAction");
}

AbstractAction::~AbstractAction()
{
    delete d;
}

QAction* AbstractAction::qAction() const
{
    return d->action;
}

void AbstractAction::setShortCutInfo(const QString& id, const QString& description, const QKeySequence& defaultShortCut)
{
    d->hasShortCut = true;
    d->id = id;
    d->description = description;
    if (!defaultShortCut.isEmpty() && d->action->shortcut().isEmpty()) {
        d->action->setShortcut(defaultShortCut);
    }
    d->keySequence = defaultShortCut.isEmpty() ? d->action->shortcut() : defaultShortCut;
}

bool AbstractAction::hasShortCut() const
{
    return d->hasShortCut;
}

QString AbstractAction::id() const
{
    return d->id;
}

QString AbstractAction::description() const
{
    return d->description;
}

QKeySequence AbstractAction::keySequence() const
{
    return d->keySequence;
}
