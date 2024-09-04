// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanagermodule.h"
#include "common/actionmanager/actionmanager.h"
#include "common/actionmanager/actioncontainer.h"
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
    auto mHelp = ActionManager::instance()->actionContainer(M_HELP);
    mHelp->appendGroup("Help.Group.Plugin");
    mHelp->addSeparator("Help.Group.Plugin");

    QAction *aboutPluginAction = new QAction(MWM_ABOUT_PLUGINS, this);
    auto cmd = ActionManager::instance()->registerAction(aboutPluginAction, "Help.AboutPlugins");
    mHelp->addAction(cmd, "Help.Group.Plugin");

    QAction *navigationItemAction = new QAction(MWMTA_PLUGINS, this);
    navigationItemAction->setIcon(QIcon::fromTheme("plugins-navigation"));
    auto actionOptionsImpl = new AbstractAction(navigationItemAction, this);
    actionOptionsImpl->setShortCutInfo("Tools.Plugins",
                                       MWMTA_PLUGINS);
    uiController->addNavigationItem(actionOptionsImpl, Priority::lowest);

    std::function<AbstractWidget *()> detailWidgetCreator = [this]() -> AbstractWidget * {
        if (!pluginsUi) {
            pluginsUi = new PluginsUi();
        }
        return new AbstractWidget(pluginsUi->getPluginDetailView());
    };

    std::function<AbstractWidget *()> storeWidgetCreator = [this]() -> AbstractWidget * {
        if (!pluginsUi) {
            pluginsUi = new PluginsUi();
        }
        return new AbstractWidget(pluginsUi->getStoreWidget());
    };

    uiController->registerWidgetCreator("pluginDetail", detailWidgetCreator);
    uiController->registerWidgetCreator(MWMTA_PLUGINS, storeWidgetCreator);

    uiController->bindWidgetToNavigation(MWMTA_PLUGINS, actionOptionsImpl);

    QObject::connect(navigationItemAction, &QAction::triggered, this, [this]() {
        uiController->showWidgetAtPosition("pluginDetail", Position::Central);
        uiController->showWidgetAtPosition(MWMTA_PLUGINS, Position::Left);
        auto windowService = dpfGetService(dpfservice::WindowService);
        if (windowService)
            windowService->setDockHeaderName(MWMTA_PLUGINS, tr("Extensions"));
    });
    QObject::connect(aboutPluginAction, &QAction::triggered, this, [this]() { uiController->switchWidgetNavigation(MWMTA_PLUGINS); });
}
