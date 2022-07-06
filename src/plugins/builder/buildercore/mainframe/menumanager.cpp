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
#include "menumanager.h"

#include "base/abstractmenu.h"
#include "services/window/windowservice.h"
#include "services/window/windowelement.h"
#include "common/common.h"

#include <QMenu>

class MenuManagerPrivate{
    friend class MenuManager;

    QSharedPointer<QAction> buildAction;
    QSharedPointer<QAction> rebuildAction;
    QSharedPointer<QAction> cleanAction;
};

MenuManager::MenuManager(dpfservice::WindowService *windowService, QObject *parent)
    : QObject(parent)
    , d(new MenuManagerPrivate())
{
    initialize(windowService);
}

void MenuManager::initialize(dpfservice::WindowService *windowService)
{
    if (!windowService)
        return;

    auto actionInit = [windowService](QAction *action, QString actionID, QString menuID, QKeySequence key){
        ActionManager::getInstance()->registerAction(action, actionID,
                                                     action->text(), key);
        AbstractAction *actionImpl = new AbstractAction(action);
        windowService->addAction(menuID, actionImpl);
    };

    d->buildAction.reset(new QAction("Build"));
    actionInit(d->buildAction.get(), "Build.Build", dpfservice::MWM_BUILD, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_B));

    d->rebuildAction.reset(new QAction("Rebuild"));
    actionInit(d->rebuildAction.get(), "Build.Rebuild", dpfservice::MWM_BUILD, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_B));

    d->cleanAction.reset(new QAction("Clean"));
    actionInit(d->cleanAction.get(), "Build.Clean", dpfservice::MWM_BUILD, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_C));
}

QSharedPointer<QAction> MenuManager::getActionPointer(ActionType actiontype)
{
    switch (actiontype) {
    case ActionType::build:
        return d->buildAction;
    case ActionType::rebuild:
        return d->rebuildAction;
    case ActionType::clean:
        return d->cleanAction;
    }
    return nullptr;
}

void MenuManager::handleRunStateChanged(BuildState state)
{
    switch (state) {
    case BuildState::kNoBuild:
    case BuildState::kBuildFailed:
        d->buildAction->setEnabled(true);
        break;
    case BuildState::kBuilding:
        d->buildAction->setEnabled(false);
        break;
    }
}
