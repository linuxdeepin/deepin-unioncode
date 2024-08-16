// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACTIONMANAGER_P_H
#define ACTIONMANAGER_P_H

#include "actionmanager.h"

#include <QHash>
#include <QSet>
#include <QSettings>

class ActionManagerPrivate : public QObject
{
    Q_OBJECT
public:
    ActionManagerPrivate();
    ~ActionManagerPrivate();

    void setContext(const QStringList &context);
    bool hasContext(const QStringList &context) const;

    Command *overridableAction(const QString &id);
    void scheduleContainerUpdate(ActionContainer *actionContainer);
    void updateContainer();
    void containerDestroyed(QObject *sender);
    QWidget *mainWindow() const;

    void saveSettings();
    void saveSettings(Command *cmd);
    void readUserSettings(const QString &id, Command *cmd);

public:
    using IdCmdMap = QHash<QString, Command *>;
    using IdContainerMap = QHash<QString, ActionContainer *>;

    IdCmdMap idCmdMap;
    IdContainerMap idContainerMap;
    QSet<ActionContainer *> scheduledContainerUpdates;
    QStringList cmdContext;
    QSettings settings;
};

#endif   // ACTIONMANAGER_P_H
