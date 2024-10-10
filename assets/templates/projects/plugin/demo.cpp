// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "demo.h"

#include "common/common.h"
#include "common/actionmanager/actionmanager.h"
#include "common/actionmanager/actioncontainer.h"
#include "services/window/windowservice.h"

#include "base/abstractwidget.h"
#include "base/abstractaction.h"
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

    // add menu
    auto mTools = ActionManager::instance()->actionContainer(M_TOOLS);
    auto demoMenu = ActionManager::instance()->createContainer("demoMenu");
    demoMenu->menu()->setTitle("demo menu");
    QAction *demoAction = new QAction("demo", this);
    auto cmd = ActionManager::instance()->registerAction(demoAction, "demo.demo");
    demoMenu->addAction(cmd);
    mTools->addMenu(demoMenu);

    if (windowService) {
        QString contextTitle = "DemoContext";
        QString dockTitle = "DemoDock";
        QString mainWindowTitle = "DemoMainWindow";
        QString displayText = "Hello UnionCode!";

        QAction *action = new QAction(mainWindowTitle, this);
        action->setIcon(QIcon::fromTheme(":/demo/images/navigation.png"));
        // Add navigation to navigationbar
        windowService->addNavigationItem(new AbstractAction(action), Priority::lowest);

        // Add widget to bottom window
        QLabel *contextLabel = new QLabel(displayText);
        contextLabel->setFont(QFont("", 100, 100, true));
        windowService->addContextWidget(contextTitle, new AbstractWidget(contextLabel), true);

        // Register mainWindow
        QLabel *mainLabel = new QLabel(displayText);
        auto impl = new AbstractWidget(mainLabel);
        mainLabel->setFont(QFont("", 100, 100, true));
        windowService->registerWidget(mainWindowTitle, impl);

        // register left dock
        QLabel *dockLabel = new QLabel(displayText);
        impl = new AbstractWidget(dockLabel);
        windowService->registerWidget(dockTitle, impl);

        // activated when triggered navigation
        connect(action, &QAction::triggered, this, [=](){
            // show mainwindow
            windowService->showWidgetAtPosition(mainWindowTitle, dpfservice::Position::Central, true);

            // show contextWidget
            windowService->showContextWidget();
            windowService->switchContextWidget(contextTitle);

            // show left dock
            windowService->setDockHeaderName(dockTitle, "demoDockHeader");
            windowService->showWidgetAtPosition(dockTitle, dpfservice::Position::Left, true);

            sendEvent(); //test
        }, Qt::DirectConnection);
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

