// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACTIONCONTAINER_H
#define ACTIONCONTAINER_H

#include <QObject>

class QMenu;
class QAction;
class Command;
class ActionManagerPrivate;
class ActionContainerPrivate;
class ActionContainer : public QObject
{
    Q_OBJECT
public:
    enum OnAllDisabledBehavior {
        Disable,
        Hide,
        Show
    };

    explicit ActionContainer(const QString &id, ActionManagerPrivate *actMngPrivate);
    ~ActionContainer();

    void setOnAllDisabledBehavior(OnAllDisabledBehavior behavior);
    ActionContainer::OnAllDisabledBehavior onAllDisabledBehavior() const;

    QString id() const;
    QMenu *menu() const;
    QAction *containerAction() const;

    void appendGroup(const QString &groupId);
    void insertGroup(const QString &before, const QString &groupId);
    void addAction(Command *action, const QString &groupId = {});
    void addMenu(ActionContainer *menu, const QString &groupId = {});
    void addMenu(ActionContainer *before, ActionContainer *menu);
    Command *addSeparator(const QString &group = {});
    Command *addSeparator(const QStringList &context, const QString &group = {}, QAction **outSeparator = nullptr);
    void clear();

    QAction *actionForItem(QObject *item) const;
    void insertAction(QAction *before, Command *command);
    void insertMenu(QAction *before, ActionContainer *container);

    void removeAction(Command *command);
    void removeMenu(ActionContainer *container);

    bool update();

private:
    ActionContainerPrivate *const d;
};

#endif   // ACTIONCONTAINER_H
