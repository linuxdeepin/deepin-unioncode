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
#include "base/abstractmenu.h"
#include "services/window/windowservice.h"
#include "buildmanager.h"

#include <QMenu>

using namespace dpfservice;
MenuManager::MenuManager(QObject *parent) : QObject(parent)
{
}

void MenuManager::initialize(WindowService *windowService)
{
    if (!windowService)
        return;

    // insert build action.
    buildAction.reset(new QAction("Build"));
    buildAction->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_B));
    AbstractAction *actionImpl = new AbstractAction(buildAction.get());
    windowService->addAction(QString::fromStdString(MENU_BUILD), actionImpl);
    // triggered by top menu.
    connect(buildAction.get(), &QAction::triggered, BuildManager::instance(), &BuildManager::buildProject, Qt::DirectConnection);
}

void MenuManager::handleRunStateChanged(BuildManager::BuildState state)
{
    switch (state) {
    case BuildManager::kNoBuild:
        buildAction->setEnabled(true);
        break;
    case BuildManager::kBuilding:
        buildAction->setEnabled(false);
        break;
    }
}
