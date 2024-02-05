// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanagermodule.h"
#include "common/actionmanager/actionmanager.h"
#include "services/window/windowelement.h"
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
    ActionManager::getInstance()->registerAction(pluginManagerAction, "Help.AboutPlugins", MWM_ABOUT_PLUGINS, QKeySequence());

    auto actionOptionsImpl = new AbstractAction(pluginManagerAction);
    actionOptionsImpl->setShortCutInfo("Tools.Plugins",
                                       MWMTA_PLUGINS);

    uiController->addAction(MWM_HELP, actionOptionsImpl);
    uiController->addNavigationItem(actionOptionsImpl);
    QObject::connect(pluginManagerAction, &QAction::triggered, this, [this](){
        uiController->raiseMode(CM_EDIT);
        uiController->replaceWidget("pluginDetail", new AbstractWidget(pluginsUi->getPluginDetailView()), Position::FullWindow);
        uiController->replaceWidget(MWMTA_PLUGINS, new AbstractWidget(pluginsUi->getPluginView()), Position::Left);
    });
}
