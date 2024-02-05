// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanagermodule.h"
#include "common/actionmanager/actionmanager.h"
#include "services/window/windowelement.h"
#include "base/abstractaction.h"
#include "uicontroller/controller.h"
#include "gui/plugindialog.h"

#include <QAction>

using namespace dpfservice;

PluginManagerModule::PluginManagerModule()
{

    pluginDialog = new PluginDialog();
}

PluginManagerModule::~PluginManagerModule()
{
    if (pluginDialog) {
        delete pluginDialog;
        pluginDialog = nullptr;
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
    QObject::connect(pluginManagerAction, &QAction::triggered,
                     pluginDialog, &QDialog::show);
}
