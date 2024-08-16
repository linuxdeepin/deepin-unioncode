// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "command.h"
#include "command_p.h"
#include "commandaction.h"
#include "action_define.h"

#include <QTextStream>

static QString msgActionWarning(QAction *newAction, const QString &id, QAction *oldAction)
{
    QString msg;
    QTextStream str(&msg);
    str << "addOverrideAction " << newAction->objectName() << '/' << newAction->text()
        << ": Action ";
    if (oldAction)
        str << oldAction->objectName() << '/' << oldAction->text();
    str << " is already registered for context " << id << '.';
    return msg;
}

CommandPrivate::CommandPrivate(const QString &id, Command *qq)
    : q(qq),
      cmdId(id),
      action(new CommandAction(qq))
{
    action->setShortcutVisibleInToolTip(true);
    connect(action, &QAction::changed, this, &CommandPrivate::updateActiveState);
}

void CommandPrivate::setCurrentContext(const QStringList &context)
{
    cmdContext = context;

    QAction *currentAction = nullptr;
    for (const auto &id : std::as_const(context)) {
        if (id == C_GLOBAL_CUTOFF)
            break;

        if (QAction *a = contextActionMap.value(id, nullptr)) {
            currentAction = a;
            break;
        }
    }

    action->setAction(currentAction);
    updateActiveState();
}

void CommandPrivate::addOverrideAction(QAction *action, const QStringList &context)
{
    if (action->menuRole() == QAction::TextHeuristicRole)
        action->setMenuRole(QAction::NoRole);

    if (isEmpty())
        this->action->initialize(action);

    if (context.isEmpty()) {
        contextActionMap.insert(C_GLOBAL, action);
    } else {
        for (const auto &id : context) {
            if (contextActionMap.contains(id))
                qWarning("%s", qPrintable(msgActionWarning(action, id, contextActionMap.value(id, nullptr))));
            contextActionMap.insert(id, action);
        }
    }

    setCurrentContext(cmdContext);
}

void CommandPrivate::removeOverrideAction(QAction *action)
{
    auto iter = contextActionMap.begin();
    for (; iter != contextActionMap.end();) {
        if (!iter.value() || iter.value() == action)
            iter = contextActionMap.erase(iter);
        else
            ++iter;
    }
}

bool CommandPrivate::isEmpty() const
{
    return contextActionMap.isEmpty();
}

void CommandPrivate::updateActiveState()
{
    setActive(action->isEnabled() && action->isVisible() && !action->isSeparator());
}

void CommandPrivate::setActive(bool state)
{
    if (state == active)
        return;

    active = state;
    emit q->activeStateChanged();
}

Command::Command(const QString &id, QObject *parent)
    : QObject(parent),
      d(new CommandPrivate(id, this))
{
}

Command::~Command()
{
    delete d;
}

void Command::setDefaultKeySequence(const QKeySequence &key)
{
    if (!d->isKeyInitialized)
        setKeySequences({ key });
    d->defaultKeys = { key };
}

void Command::setDefaultKeySequences(const QList<QKeySequence> &keys)
{
    if (!d->isKeyInitialized)
        setKeySequences(keys);
    d->defaultKeys = keys;
}

void Command::setKeySequences(const QList<QKeySequence> &keys)
{
    d->isKeyInitialized = true;
    d->action->setShortcuts(keys);
    emit keySequenceChanged();
}

QList<QKeySequence> Command::defaultKeySequences() const
{
    return d->defaultKeys;
}

QList<QKeySequence> Command::keySequences() const
{
    return d->action->shortcuts();
}

QKeySequence Command::keySequence() const
{
    return d->action->shortcut();
}

void Command::setDescription(const QString &text)
{
    d->defaultText = text;
}

QString Command::description() const
{
    if (!d->defaultText.isEmpty())
        return d->defaultText;
    if (QAction *act = action()) {
        const QString text = stripAccelerator(act->text());
        if (!text.isEmpty())
            return text;
    }
    return d->cmdId;
}

QString Command::id() const
{
    return d->cmdId;
}

QAction *Command::action() const
{
    return d->action;
}

QStringList Command::context() const
{
    return d->cmdContext;
}

void Command::setAttribute(CommandAttribute attr)
{
    d->attributes |= attr;
    switch (attr) {
    case Command::CA_Hide:
        d->action->setAttribute(CommandAction::Hide);
        break;
    case Command::CA_UpdateText:
        d->action->setAttribute(CommandAction::UpdateText);
        break;
    case Command::CA_UpdateIcon:
        d->action->setAttribute(CommandAction::UpdateIcon);
        break;
    case Command::CA_NonConfigurable:
        break;
    }
}

void Command::removeAttribute(CommandAttribute attr)
{
    d->attributes &= ~attr;
    switch (attr) {
    case Command::CA_Hide:
        d->action->removeAttribute(CommandAction::Hide);
        break;
    case Command::CA_UpdateText:
        d->action->removeAttribute(CommandAction::UpdateText);
        break;
    case Command::CA_UpdateIcon:
        d->action->removeAttribute(CommandAction::UpdateIcon);
        break;
    case Command::CA_NonConfigurable:
        break;
    }
}

bool Command::hasAttribute(CommandAttribute attr) const
{
    return d->attributes.testFlag(attr);
}

bool Command::isActive() const
{
    return d->active;
}
