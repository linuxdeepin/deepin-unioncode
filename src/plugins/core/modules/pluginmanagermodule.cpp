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
#include "gui/plugindetailsview.h"
#include "gui/pluginstorewidget.h"

#include <QAction>

using namespace dpfservice;

PluginManagerModule::PluginManagerModule()
{
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

    uiController->addAction(MWM_HELP, actionOptionsImpl);
    uiController->addNavigationItem(actionOptionsImpl, Priority::lowest);

    std::function<AbstractWidget*()> detailWidgetCreator = [this]()->AbstractWidget*{
        if (!pluginsUi) {
            pluginsUi = new PluginsUi();
        } 
        return new AbstractWidget(pluginsUi->getPluginDetailView());
    };
    
    std::function<AbstractWidget*()> storeWidgetCreator = [this]()->AbstractWidget*{
        if (!pluginsUi) {
            pluginsUi = new PluginsUi();
        } 
        return new AbstractWidget(pluginsUi->getStoreWidget());
    };

    uiController->registerWidgetCreator("pluginDetail", detailWidgetCreator);
    uiController->registerWidgetCreator(MWMTA_PLUGINS, storeWidgetCreator);
    
    uiController->bindWidgetToNavigation(MWMTA_PLUGINS, actionOptionsImpl);

    QObject::connect(pluginManagerAction, &QAction::triggered, this, [this]() {
        uiController->showWidgetAtPosition("pluginDetail", Position::Central);
        uiController->showWidgetAtPosition(MWMTA_PLUGINS, Position::Left);
        auto windowService = dpfGetService(dpfservice::WindowService);
        if (windowService)
            windowService->setDockHeaderName(MWMTA_PLUGINS, tr("Extensions"));
    });
}
