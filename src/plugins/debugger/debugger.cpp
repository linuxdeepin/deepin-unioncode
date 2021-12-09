/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "debugger.h"
#include "base/abstractnav.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"
#include "base/abstractmainwindow.h"
#include "services/window/windowservice.h"

#include <QMenu>

using namespace dpfservice;

void Debugger::initialize()
{

}

bool Debugger::start()
{
    auto &ctx = dpfInstance.serviceContext();
    qInfo() << "import service list" <<  ctx.services();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    QMenu *menu = new QMenu();
    AbstractMenu *menuImpl = new AbstractMenu(menu);
    windowService->addMenu(menuImpl);

    if (menu->menuAction()) {
        menu->menuAction()->setText("Debuger Plugin Test");
    }

    QAction *action = new QAction("setp");
    AbstractAction * actionImpl = new AbstractAction(action);
    windowService->addAction(QString::fromStdString(MENU_DEBUG), actionImpl);

    return true;
}

dpf::Plugin::ShutdownFlag Debugger::stop()
{
    return Sync;
}
