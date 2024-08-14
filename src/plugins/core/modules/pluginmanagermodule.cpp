// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanagermodule.h"
#include "common/actionmanager/actionmanager.h"
#include "services/window/windowelement.h"
#include "services/window/windowservice.h"
#include "base/abstractaction.h"
#include "uicontroller/controller.h"
#include "gui/pluginsui.h"

#include <QAction>

using namespace dpfservice;

PluginManagerModule::PluginManagerModule()
{
    pluginsUi = new PluginsUi();
}

PluginManagerModule::~PluginManagerModule()
{
    if (pluginsUi) {
        delete pluginsUi;
        pluginsUi = nullptr;
    }
}

void PluginManagerModule::initialize(Controller *_uiController)
{
    AbstractModule::initialize(_uiController);

    QAction *pluginManagerAction = new QAction(MWMTA_PLUGINS, this);
    pluginManagerAction->setIcon(QIcon::fromTheme("plugins-navigation"));
    ActionManager::getInstance()->registerAction(
            pluginManagerAction, "Help.AboutPlugins", MWM_ABOUT_PLUGINS, QKeySequence());

    auto actionOptionsImpl = new AbstractAction(pluginManagerAction, this);
    actionOptionsImpl->setShortCutInfo("Tools.Plugins",
                                       MWMTA_PLUGINS);

    auto menuAction = new QAction(MWMTA_PLUGINS, this);
    menuAction->setIcon(QIcon::fromTheme("plugins-navigation"));
    uiController->addAction(MWM_HELP, new AbstractAction(menuAction));
    uiController->addNavigationItem(actionOptionsImpl, Priority::lowest);

    auto detailViewImpl = new AbstractWidget(pluginsUi->getPluginDetailView());
    auto storeWidgetImpl = new AbstractWidget(pluginsUi->getStoreWidget());

    uiController->registerWidget("pluginDetail", detailViewImpl);
    uiController->registerWidget(MWMTA_PLUGINS, storeWidgetImpl);
    uiController->bindWidgetToNavigation(MWMTA_PLUGINS, actionOptionsImpl);

    QObject::connect(pluginManagerAction, &QAction::triggered, this, [this]() {
        uiController->showWidgetAtPosition("pluginDetail", Position::Central);
        uiController->showWidgetAtPosition(MWMTA_PLUGINS, Position::Left);
        auto windowService = dpfGetService(dpfservice::WindowService);
        if (windowService)
            windowService->setDockHeaderName(MWMTA_PLUGINS, tr("Extensions"));
    });
    QObject::connect(menuAction, &QAction::triggered, this, [this](){uiController->switchWidgetNavigation(MWM_ABOUT_PLUGINS);});
}
