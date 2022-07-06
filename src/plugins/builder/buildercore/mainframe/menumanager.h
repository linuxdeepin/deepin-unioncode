/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include "services/builder/builderglobals.h"

#include <QObject>
#include <QAction>

enum ActionType
{
    build = 0,
    rebuild,
    clean
};

namespace dpfservice {
class WindowService;
}

class MenuManagerPrivate;
class MenuManager : public QObject
{
    Q_OBJECT
public:
    explicit MenuManager(dpfservice::WindowService *service, QObject *parent = nullptr);
    QSharedPointer<QAction> getActionPointer(ActionType actiontype);

signals:

public slots:
    void handleRunStateChanged(BuildState state);

private:    
    void initialize(dpfservice::WindowService *service);
    MenuManagerPrivate *const d;
};

#endif // MENUMANAGER_H
