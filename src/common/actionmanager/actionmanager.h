// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include "action_define.h"
#include "command.h"
#include "actioncontainer.h"

#include <QObject>

class QAction;
class ActionManagerPrivate;
class ActionManager : public QObject
{
    Q_OBJECT
public:
    static ActionManager *instance();

    ActionContainer *createContainer(const QString &containerId);
    ActionContainer *actionContainer(const QString &containerId);

    /**
     * @brief registerAction
     * @param action : A QAction object
     * @param id: The action id
     * @param context: Used to control the active state of the `action`, The default value is `C_GLOBAL`.
     *                 If it is a custom context, need to call `addContext` to add it first
     * @return Command: Gets or sets information such as shortcut keys
     */
    Command *registerAction(QAction *action, const QString &id,
                            const QStringList &context = { C_GLOBAL });
    void unregisterAction(QAction *action, const QString &id);
    Command *command(const QString &id);
    QList<Command *> commandList();

    void addContext(const QStringList &context);
    void removeContext(const QStringList &context);
    void setContext(const QStringList &context);
    bool hasContext(const QStringList &context);
    QStringList context() const;

    void saveSettings();

signals:
    void commandListChanged();
    void commandAdded(const QString &id);

private:
    ActionManager(QObject *parent = nullptr);
    ~ActionManager() override;

    ActionManagerPrivate *const d;
    friend class Controller;
};

#endif   // ACTIONMANAGER_H
