// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "demo.h"

#include "common/common.h"
#include "services/window/windowservice.h"

#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractmenu.h"

#include <QAction>
#include <QIcon>

const char *D_TSET = "test";

void sendEvent();

void Demo::initialize()
{
}

bool Demo::start()
{
    auto windowService = dpfGetService(dpfservice::WindowService);

    if (windowService) {
        QString title = "Demo";
        QString displayText = "Hello UnionCode!";

        // Add widget to bottom window
        if (windowService->addContextWidget) {
            QLabel *contextLabel = new QLabel(displayText);
            contextLabel->setFont(QFont("", 100, 100, true));
            windowService->addContextWidget(title, new AbstractWidget(contextLabel), title);
        }

        // Add widget to left bar
        if (windowService->addCentralNavigation) {
            QLabel *navigationLabel = new QLabel(displayText);
            navigationLabel->setFont(QFont("", 100, 100, true));
            windowService->addActionNavigation(title, new AbstractAction(new QAction(QIcon(":/demo/images/navigation.png"), QAction::tr("Demo"))));
            windowService->addCentralNavigation(title, new AbstractCentral(navigationLabel));
        }

        // Add menu
        if (windowService->addMenu) {
            QMenu *menu = new QMenu(title);
            QAction *action = new QAction("test event", menu);
            connect(action, &QAction::triggered, [=](){
                // send event to eventreceiver
                sendEvent();
            });
            menu->addAction(action);
            windowService->addMenu(new AbstractMenu(menu));
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag Demo::stop()
{
    return Sync;
}

void sendEvent()
{
    dpf::Event event;
    event.setTopic(T_MENU);
    event.setData(D_TSET);
    event.setProperty(P_ACTION_TEXT, "Event has been received successfully!");
    dpf::EventCallProxy::instance().pubEvent(event);
}
