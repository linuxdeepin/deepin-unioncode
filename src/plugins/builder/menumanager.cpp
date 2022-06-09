/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "buildmanager.h"

#include "base/abstractmenu.h"
#include "services/window/windowservice.h"
#include "common/common.h"

#include <QMenu>

using namespace dpfservice;
MenuManager::MenuManager(QObject *parent) : QObject(parent)
{
}

void MenuManager::initialize(WindowService *windowService)
{
    if (!windowService)
        return;

    auto actionInit = [windowService](QAction *action, QString actionID, QString menuID, QKeySequence key){
        ActionManager::getInstance()->registerAction(action, actionID,
                                                     action->text(), key);
        AbstractAction *actionImpl = new AbstractAction(action);
        windowService->addAction(menuID, actionImpl);
    };

    buildAction.reset(new QAction("Build"));
    actionInit(buildAction.get(), "Build.Build", MWM_BUILD, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_B));

    rebuildAction.reset(new QAction("Rebuild"));
    actionInit(rebuildAction.get(), "Build.Rebuild", MWM_BUILD, QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_B));

    cleanAction.reset(new QAction("Clean"));
    actionInit(cleanAction.get(), "Build.Clean", MWM_BUILD, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_C));

    // triggered by top menu.
    connect(buildAction.get(), &QAction::triggered, BuildManager::instance(), &BuildManager::buildProject, Qt::DirectConnection);
    connect(rebuildAction.get(), &QAction::triggered, BuildManager::instance(), &BuildManager::rebuildProject, Qt::DirectConnection);
    connect(cleanAction.get(), &QAction::triggered, BuildManager::instance(), &BuildManager::cleanProject, Qt::DirectConnection);
}

void MenuManager::handleRunStateChanged(BuildManager::BuildState state)
{
    switch (state) {
    case BuildManager::kNoBuild:
    case BuildManager::kBuildFailed:
        buildAction->setEnabled(true);
        break;
    case BuildManager::kBuilding:
        buildAction->setEnabled(false);
        break;
    }
}
