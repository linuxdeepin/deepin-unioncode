// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recent.h"
#include "base/abstractmenu.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"
#include "mainframe/recentdisplaywidget.h"
#include "transceiver/recentreceiver.h"

#include <QAction>
#include <QLabel>

using namespace dpfservice;

void Recent::initialize()
{

}

bool Recent::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        QObject::connect(RecentProxy::instance(), &RecentProxy::saveOpenedProject,
                         RecentDisplayWidget::instance(), &RecentDisplayWidget::addProject);
        QObject::connect(RecentProxy::instance(), &RecentProxy::saveOpenedFile,
                         RecentDisplayWidget::instance(), &RecentDisplayWidget::addDocument);

        QAction *action = new QAction(MWNA_RECENT, this);
        action->setIcon(QIcon::fromTheme("recent-navigation"));
        windowService->addNavigationItem(new AbstractAction(action), Priority::highest);

        auto recentWidgetImpl = new AbstractWidget(RecentDisplayWidget::instance());
        windowService->registerWidgetToMode("recentWindow", recentWidgetImpl, CM_RECENT, Position::FullWindow, true, true);
    }
    return true;
}

dpf::Plugin::ShutdownFlag Recent::stop()
{
    return Sync;
}
