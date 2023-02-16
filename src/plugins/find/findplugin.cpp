/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *             hongjinchuan<hongjinchuan@uniontech.com>
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
#include "findplugin.h"

#include "services/window/windowservice.h"
#include "findtoolbar.h"
#include "findtoolwindow.h"
#include "searchresultwindow.h"
#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"

#include <QMenu>
#include <QAction>

using namespace dpfservice;

void FindPlugin::initialize()
{
    qInfo() << __FUNCTION__;

}

bool FindPlugin::start()
{
    qInfo() << __FUNCTION__;

    auto &ctx = dpfInstance.serviceContext();
    windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    QMenu* editMenu = new QMenu(QMenu::tr("&Edit"));
    QAction* findAction = new QAction();
    QAction* advancedFindAction = new QAction();


    ActionManager::getInstance()->registerAction(findAction, "Edit.Find",
                                                 tr("Find/Replace"), QKeySequence(Qt::Modifier::CTRL | Qt::Key_F),
                                                 "find.png");
    ActionManager::getInstance()->registerAction(advancedFindAction, "Edit.Advanced.Find",
                                                 tr("Advanced Find"), QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key_F),
                                                 "edit-find.png");

    editMenu->addAction(findAction);
    editMenu->addAction(advancedFindAction);
    windowService->addToolBarActionItem("Find/Replace", findAction);
    windowService->addToolBarSeparator();

    connect(findAction, &QAction::triggered, [=] {
        emit windowService->showFindToolBar();
    });

    connect(advancedFindAction, &QAction::triggered, [=] {
        editor.switchContext(tr("Advanced &Search"));
    });

    AbstractMenu * menuImpl = new AbstractMenu(editMenu);
    windowService->addMenu(menuImpl);

    AbstractWidget *widgetImpl = new AbstractWidget(new FindToolWindow());
    emit windowService->addContextWidget(tr("Advanced &Search"), widgetImpl, "Advsearch");

    FindToolBar * findToolBar = new FindToolBar();
    AbstractWidget *abstractFindToolBar = new AbstractWidget(findToolBar);
    emit windowService->addFindToolBar(abstractFindToolBar);
    connect(findToolBar, &FindToolBar::advanced, this, &FindPlugin::sendSwitchSearchResult);

    return true;
}

void FindPlugin::sendSwitchSearchResult()
{
    emit editor.switchContext(tr("Advanced &Search"));
}

dpf::Plugin::ShutdownFlag FindPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


