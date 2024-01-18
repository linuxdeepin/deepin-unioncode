// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findplugin.h"

#include "services/window/windowservice.h"
#include "findtoolbar.h"
#include "findtoolwindow.h"
#include "searchresultwindow.h"
#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"

#include <DMenu>
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

    DMenu* editMenu = new DMenu(QMenu::tr("&Edit"));
    QAction* findAction = new QAction(this);
    QAction* advancedFindAction = new QAction(this);


    ActionManager::getInstance()->registerAction(findAction, "Edit.Find",
                                                 tr("Find/Replace"), QKeySequence(Qt::Modifier::CTRL | Qt::Key_F),
                                                 "search-find");
    ActionManager::getInstance()->registerAction(advancedFindAction, "Edit.Advanced.Find",
                                                 tr("Advanced Find"), QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key_F),
                                                 "search-find");

    editMenu->addAction(findAction);
    editMenu->addAction(advancedFindAction);

    windowService->addTopToolItem("Edit.Find", new AbstractAction(findAction), MWNA_EDIT);
    windowService->addTopToolItem("Edit.Find", new AbstractAction(findAction), MWNA_DEBUG);
    windowService->addTopToolSpacing("Edit.Find", 20);

    connect(advancedFindAction, &QAction::triggered, [=] {
        uiController.switchContext(tr("Advanced &Search"));
    });

    AbstractMenu * menuImpl = new AbstractMenu(editMenu);
    windowService->addChildMenu(menuImpl);

    AbstractWidget *widgetImpl = new AbstractWidget(new FindToolWindow());
    windowService->addContextWidget(tr("Advanced &Search"), widgetImpl, true);

    FindToolBar * findToolBar = new FindToolBar();
    AbstractWidget *abstractFindToolBar = new AbstractWidget(findToolBar);

    connect(this, &FindPlugin::onFindActionTriggered, findToolBar, &FindToolBar::handleFindActionTriggered);
    connect(findAction, &QAction::triggered, [=] {
        if (findToolBar) {
            if (findToolBar->isVisible()) {
                findToolBar->hide();
            } else {
                findToolBar->show();
            }
        }
    });

    if(windowService->registerWidgetToMode) {
        windowService->registerWidgetToMode("findWidget", abstractFindToolBar, CM_EDIT, Position::Top, true, false);
        windowService->registerWidgetToMode("findWidget", abstractFindToolBar, CM_DEBUG, Position::Top, true, false);
    }
    return true;
}

void FindPlugin::sendSwitchSearchResult()
{
    uiController.switchContext(tr("Advanced &Search"));
}

dpf::Plugin::ShutdownFlag FindPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


