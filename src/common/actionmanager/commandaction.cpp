// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandaction.h"
#include "action_define.h"

class CommandActionPrivate : public QObject
{
public:
    CommandActionPrivate(CommandAction *qq);

    void actionChanged();
    void updateState();
    void updateToolTipWithKeySequence();
    void disconnectAction();
    void connectAction();
    void update(QAction *action, bool initialize);

public:
    CommandAction *q;

    QPointer<QAction> action;
    CommandAction::Attributes attributes;
    bool showShortcut = false;
    QString toolTip;
    bool block = false;
};

CommandActionPrivate::CommandActionPrivate(CommandAction *qq)
    : q(qq)
{
}

void CommandActionPrivate::actionChanged()
{
    update(action, false);
}

void CommandActionPrivate::updateState()
{
    if (action) {
        update(action, false);
    } else {
        if (q->hasAttribute(CommandAction::Hide))
            q->setVisible(false);
        q->setEnabled(false);
    }
}

void CommandActionPrivate::updateToolTipWithKeySequence()
{
    if (block)
        return;

    block = true;
    if (!showShortcut || q->shortcut().isEmpty())
        q->setToolTip(toolTip);
    else
        q->setToolTip(q->stringWithAppendedShortcut(toolTip, q->shortcut()));
    block = false;
}

void CommandActionPrivate::disconnectAction()
{
    if (action) {
        disconnect(action.data(), &QAction::changed, this, &CommandActionPrivate::actionChanged);
        disconnect(q, &CommandAction::triggered, action.data(), &QAction::triggered);
        disconnect(q, &CommandAction::toggled, action.data(), &QAction::setChecked);
    }
}

void CommandActionPrivate::connectAction()
{
    if (action) {
        connect(action.data(), &QAction::changed, this, &CommandActionPrivate::actionChanged);
        connect(q, &CommandAction::triggered, action.data(), &QAction::triggered);
        connect(q, &CommandAction::toggled, action.data(), &QAction::setChecked);
    }
}

void CommandActionPrivate::update(QAction *action, bool initialize)
{
    if (!action)
        return;

    disconnect(q, &CommandAction::changed, this, &CommandActionPrivate::updateToolTipWithKeySequence);
    if (initialize) {
        q->setSeparator(action->isSeparator());
        q->setMenuRole(action->menuRole());
    }
    if (q->hasAttribute(CommandAction::UpdateIcon) || initialize) {
        q->setIcon(action->icon());
        q->setIconText(action->iconText());
        q->setIconVisibleInMenu(action->isIconVisibleInMenu());
    }
    if (q->hasAttribute(CommandAction::UpdateText) || initialize) {
        q->setText(action->text());
        toolTip = action->toolTip();
        updateToolTipWithKeySequence();
        q->setStatusTip(action->statusTip());
        q->setWhatsThis(action->whatsThis());
    }

    q->setCheckable(action->isCheckable());
    if (!initialize) {
        if (q->isChecked() != action->isChecked()) {
            if (this->action)
                disconnect(q, &CommandAction::toggled, this->action.data(), &QAction::setChecked);
            q->setChecked(action->isChecked());
            if (this->action)
                connect(q, &CommandAction::toggled, this->action.data(), &QAction::setChecked);
        }
        q->setEnabled(action->isEnabled());
        q->setVisible(action->isVisible());
    }
    connect(q, &CommandAction::changed, this, &CommandActionPrivate::updateToolTipWithKeySequence);
}

CommandAction::CommandAction(QObject *parent)
    : QAction(parent),
    d(new CommandActionPrivate(this))
{
    connect(this, &QAction::changed, d, &CommandActionPrivate::updateToolTipWithKeySequence);
    d->updateState();
}

CommandAction::~CommandAction()
{
    delete d;
}

void CommandAction::initialize(QAction *action)
{
    d->update(action, true);
}

void CommandAction::setAction(QAction *action)
{
    if (d->action == action)
        return;

    d->disconnectAction();
    d->action = action;
    d->connectAction();
    d->updateState();
    emit currentActionChanged(action);
}

QAction *CommandAction::action() const
{
    return d->action;
}

bool CommandAction::shortcutVisibleInToolTip() const
{
    return d->showShortcut;
}

void CommandAction::setShortcutVisibleInToolTip(bool visible)
{
    d->showShortcut = visible;
    d->updateToolTipWithKeySequence();
}

void CommandAction::setAttribute(Attribute attribute)
{
    d->attributes |= attribute;
    d->updateState();
}

void CommandAction::removeAttribute(Attribute attribute)
{
    d->attributes &= ~attribute;
    d->updateState();
}

bool CommandAction::hasAttribute(Attribute attribute)
{
    return d->attributes.testFlag(attribute);
}

QString CommandAction::stringWithAppendedShortcut(const QString &str, const QKeySequence &shortcut)
{
    const QString s = stripAccelerator(str);
    return QString::fromLatin1("<div style=\"white-space:pre\">%1 "
                               "<span style=\"color: gray; font-size: small\">%2</span></div>")
            .arg(s, shortcut.toString(QKeySequence::NativeText));
}

CommandAction *CommandAction::commandActionWithIcon(QAction *original, const QIcon &newIcon)
{
    auto proxyAction = new CommandAction(original);
    proxyAction->setAction(original);
    proxyAction->setIcon(newIcon);
    proxyAction->setAttribute(UpdateText);
    return proxyAction;
}
