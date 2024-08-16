// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actioncontainer.h"
#include "actionmanager_p.h"
#include "command.h"
#include "action_define.h"
#include "util/qtcassert.h"

#include <QMenu>
#include <QDebug>

struct Group
{
    Group(const QString &id)
        : id(id) {}
    QString id;
    QList<QObject *> items;   // Command * or ActionContainer *
};

class ActionContainerPrivate : public QObject
{
public:
    explicit ActionContainerPrivate(ActionContainer *qq, const QString &id, ActionManagerPrivate *actMngPrivate);
    ~ActionContainerPrivate();

    void scheduleUpdate();
    void itemDestroyed(QObject *sender);
    QList<Group>::const_iterator findGroup(const QString &groupId) const;
    QAction *insertLocation(QList<Group>::const_iterator group) const;

public:
    ActionContainer *q;

    QMenu *menu { nullptr };
    QString actId;
    ActionManagerPrivate *actMngPrivate { nullptr };
    QList<Group> groupList;
    ActionContainer::OnAllDisabledBehavior onAllDisabledBehavior;
};

ActionContainerPrivate::ActionContainerPrivate(ActionContainer *qq, const QString &id, ActionManagerPrivate *actMngPrivate)
    : q(qq),
      menu(new QMenu),
      actId(id),
      actMngPrivate(actMngPrivate),
      onAllDisabledBehavior(ActionContainer::Disable)
{
}

ActionContainerPrivate::~ActionContainerPrivate()
{
    delete menu;
}

void ActionContainerPrivate::scheduleUpdate()
{
    actMngPrivate->scheduleContainerUpdate(q);
}

void ActionContainerPrivate::itemDestroyed(QObject *sender)
{
    for (Group &group : groupList) {
        if (group.items.removeAll(sender) > 0)
            break;
    }
}

QList<Group>::const_iterator ActionContainerPrivate::findGroup(const QString &groupId) const
{
    auto it = groupList.cbegin();
    while (it != groupList.constEnd()) {
        if (it->id == groupId)
            break;
        ++it;
    }
    return it;
}

QAction *ActionContainerPrivate::insertLocation(QList<Group>::const_iterator group) const
{
    if (group == groupList.constEnd())
        return nullptr;

    ++group;
    while (group != groupList.constEnd()) {
        if (!group->items.isEmpty()) {
            QObject *item = group->items.first();
            QAction *action = q->actionForItem(item);
            if (action)
                return action;
        }
        ++group;
    }

    return nullptr;
}

ActionContainer::ActionContainer(const QString &id, ActionManagerPrivate *actMngPrivate)
    : d(new ActionContainerPrivate(this, id, actMngPrivate))
{
    appendGroup(G_DEFAULT_ONE);
    appendGroup(G_DEFAULT_TWO);
    appendGroup(G_DEFAULT_THREE);
    d->scheduleUpdate();
}

ActionContainer::~ActionContainer()
{
    delete d;
}

void ActionContainer::setOnAllDisabledBehavior(OnAllDisabledBehavior behavior)
{
    d->onAllDisabledBehavior = behavior;
}

ActionContainer::OnAllDisabledBehavior ActionContainer::onAllDisabledBehavior() const
{
    return d->onAllDisabledBehavior;
}

QString ActionContainer::id() const
{
    return d->actId;
}

QMenu *ActionContainer::menu() const
{
    return d->menu;
}

QAction *ActionContainer::containerAction() const
{
    return d->menu->menuAction();
}

void ActionContainer::appendGroup(const QString &groupId)
{
    d->groupList.append(Group(groupId));
}

void ActionContainer::insertGroup(const QString &before, const QString &groupId)
{
    auto it = d->groupList.begin();
    while (it != d->groupList.end()) {
        if (it->id == before) {
            d->groupList.insert(it, Group(groupId));
            break;
        }
        ++it;
    }
}

void ActionContainer::addAction(Command *action, const QString &groupId)
{
    if (!action || !action->action())
        return;

    auto actualGroupId = !groupId.isEmpty() ? groupId : G_DEFAULT_TWO;
    auto groupIt = d->findGroup(actualGroupId);
    QTC_ASSERT(groupIt != d->groupList.constEnd(), qDebug() << "Can't find group" << groupId << "in container" << id(); return );
    d->groupList[groupIt - d->groupList.constBegin()].items.append(action);
    connect(action, &Command::activeStateChanged, d, &ActionContainerPrivate::scheduleUpdate);
    connect(action, &QObject::destroyed, d, &ActionContainerPrivate::itemDestroyed);

    QAction *beforeAction = d->insertLocation(groupIt);
    insertAction(beforeAction, action);

    d->scheduleUpdate();
}

void ActionContainer::addMenu(ActionContainer *menu, const QString &groupId)
{
    auto actualGroupId = !groupId.isEmpty() ? groupId : G_DEFAULT_TWO;
    auto groupIt = d->findGroup(actualGroupId);
    QTC_ASSERT(groupIt != d->groupList.constEnd(), return );

    d->groupList[groupIt - d->groupList.constBegin()].items.append(menu);
    connect(menu, &QObject::destroyed, d, &ActionContainerPrivate::itemDestroyed);

    QAction *beforeAction = d->insertLocation(groupIt);
    insertMenu(beforeAction, menu);

    d->scheduleUpdate();
}

void ActionContainer::addMenu(ActionContainer *before, ActionContainer *menu)
{
    for (Group &group : d->groupList) {
        const int insertionPoint = group.items.indexOf(before);
        if (insertionPoint >= 0) {
            group.items.insert(insertionPoint, menu);
            break;
        }
    }
    connect(menu, &QObject::destroyed, d, &ActionContainerPrivate::itemDestroyed);

    QAction *beforeAction = before->containerAction();
    if (beforeAction)
        insertMenu(beforeAction, menu);

    d->scheduleUpdate();
}

Command *ActionContainer::addSeparator(const QString &group)
{
    static const QStringList context { C_GLOBAL };
    return addSeparator(context, group);
}

Command *ActionContainer::addSeparator(const QStringList &context, const QString &group, QAction **outSeparator)
{
    static int separatorIdCount = 0;
    auto separator = new QAction(this);
    separator->setSeparator(true);
    auto sepId = id().append(".Separator.%1").arg(++separatorIdCount);
    Command *cmd = ActionManager::instance()->registerAction(separator, sepId, context);
    addAction(cmd, group);
    if (outSeparator)
        *outSeparator = separator;

    return cmd;
}

void ActionContainer::clear()
{
    for (Group &group : d->groupList) {
        for (QObject *item : std::as_const(group.items)) {
            if (auto command = qobject_cast<Command *>(item)) {
                removeAction(command);
                disconnect(command, &Command::activeStateChanged,
                           d, &ActionContainerPrivate::scheduleUpdate);
                disconnect(command, &QObject::destroyed, d, &ActionContainerPrivate::itemDestroyed);
            } else if (auto container = qobject_cast<ActionContainer *>(item)) {
                container->clear();
                disconnect(container, &QObject::destroyed,
                           d, &ActionContainerPrivate::itemDestroyed);
                removeMenu(container);
            }
        }
        group.items.clear();
    }

    d->scheduleUpdate();
}

QAction *ActionContainer::actionForItem(QObject *item) const
{
    if (auto cmd = qobject_cast<Command *>(item)) {
        return cmd->action();
    } else if (auto container = qobject_cast<ActionContainer *>(item)) {
        if (container->containerAction())
            return container->containerAction();
    }
    QTC_ASSERT(false, return nullptr);
}

void ActionContainer::insertAction(QAction *before, Command *command)
{
    d->menu->insertAction(before, command->action());
}

void ActionContainer::insertMenu(QAction *before, ActionContainer *container)
{
    QMenu *menu = container->menu();
    QTC_ASSERT(menu, return );
    menu->setParent(d->menu, menu->windowFlags());
    d->menu->insertMenu(before, menu);
}

void ActionContainer::removeAction(Command *command)
{
    d->menu->removeAction(command->action());
}

void ActionContainer::removeMenu(ActionContainer *container)
{
    QMenu *menu = container->menu();
    QTC_ASSERT(menu, return );
    d->menu->removeAction(menu->menuAction());
}

bool ActionContainer::update()
{
    bool hasitems = false;
    QList<QAction *> actions = d->menu->actions();

    for (const Group &group : std::as_const(d->groupList)) {
        for (QObject *item : std::as_const(group.items)) {
            if (auto container = qobject_cast<ActionContainer *>(item)) {
                actions.removeAll(container->menu()->menuAction());
                if (container == this) {
                    QByteArray warning = Q_FUNC_INFO + QByteArray(" container '");
                    if (this->menu())
                        warning += this->menu()->title().toLocal8Bit();
                    warning += "' contains itself as subcontainer";
                    qWarning("%s", warning.constData());
                    continue;
                }
                if (container->update()) {
                    hasitems = true;
                    break;
                }
            } else if (auto command = qobject_cast<Command *>(item)) {
                actions.removeAll(command->action());
                if (command->isActive()
                    && command->action()->menuRole() != QAction::ApplicationSpecificRole) {
                    hasitems = true;
                    break;
                }
            } else {
                QTC_ASSERT(false, continue);
            }
        }
        if (hasitems)
            break;
    }
    if (!hasitems) {
        // look if there were actions added that we don't control and check if they are enabled
        for (const QAction *action : std::as_const(actions)) {
            if (!action->isSeparator() && action->isEnabled()) {
                hasitems = true;
                break;
            }
        }
    }

    if (onAllDisabledBehavior() == Hide)
        d->menu->menuAction()->setVisible(hasitems);
    else if (onAllDisabledBehavior() == Disable)
        d->menu->menuAction()->setEnabled(hasitems);

    return hasitems;
}
