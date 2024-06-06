// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findplugin.h"

#include "services/window/windowservice.h"
#include "gui/findtoolwindow.h"
#include "gui/searchresultwindow.h"
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

    DMenu *editMenu = new DMenu(QMenu::tr("&Edit"));
    AbstractMenu *menuImpl = new AbstractMenu(editMenu);

    QAction *advancedFindAction = new QAction(this);
    auto advancedFindActionImpl = new AbstractAction(advancedFindAction);
    advancedFindActionImpl->setShortCutInfo("Edit.Advanced.Find",
                                            tr("Advanced Find"), QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key_F));
    connect(advancedFindAction, &QAction::triggered, [=] {
        uiController.switchContext(tr("Advanced &Search"));
    });

    menuImpl->addAction(advancedFindActionImpl);

    windowService->addChildMenu(menuImpl);

    AbstractWidget *widgetImpl = new AbstractWidget(new FindToolWindow());
    windowService->addContextWidget(tr("Advanced &Search"), widgetImpl, true);

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
