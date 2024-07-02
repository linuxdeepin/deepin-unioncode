// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include "command.h"

#include <QObject>
#include <QHash>

class ActionManagerPrivate;
class ActionManager : public QObject
{
    Q_OBJECT
public:
    static ActionManager *getInstance();

    Command *registerAction(QAction *action, const QString &id,
                            const QString &description = nullptr,
                            const QKeySequence defaultShortcut = QKeySequence());
    void unregisterAction(QString id);

    Command *command(QString id);
    QList<Command *> commands();

    void readUserSetting();
    void saveSetting();

signals:

private:
    explicit ActionManager(QObject *parent = nullptr);
    virtual ~ActionManager() override;

    ActionManagerPrivate *const d;
};

#endif // ACTIONMANAGER_H
