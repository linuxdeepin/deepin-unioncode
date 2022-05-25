/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

    Command *registerAction(QAction *action, const QString id,
                            const QString description = nullptr,
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
