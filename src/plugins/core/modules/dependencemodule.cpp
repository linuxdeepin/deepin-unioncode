// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dependencemodule.h"
#include "depend/aptinstaller.h"
#include "depend/pipinstaller.h"

#include "services/window/windowservice.h"

using namespace dpfservice;

void DependenceModule::initialize(Controller *_uiController)
{
    Q_UNUSED(_uiController)

    manager = new DependenceManager(this);
    connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, this, [=] {
        manager->setNotifiable(true);
        QTimer::singleShot(500, this, &DependenceModule::installPluginDepends);
    });

    initDefaultInstaller();
    initInterfaces();
}

void DependenceModule::initDefaultInstaller()
{
    manager->registerInstaller("apt", new APTInstaller(this));
    manager->registerInstaller("pip", new PIPInstaller(this));
}

void DependenceModule::initInterfaces()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    Q_ASSERT(windowService);

    using namespace std::placeholders;
    windowService->registerInstaller = std::bind(&DependenceManager::registerInstaller, manager, _1, _2);
    windowService->installPackages = std::bind(&DependenceManager::installPackageList, manager, _1, _2, _3, _4);
}

void DependenceModule::installPluginDepends()
{
    auto instance = dpf::LifeCycle::getPluginManagerInstance();
    auto collections = instance->pluginCollections();
    for (const auto &categories : collections.values()) {
        for (const auto &meta : categories) {
            if (!meta)
                continue;

            const auto &depends = meta->installDepends();
            for (const auto &depend : depends) {
                manager->installPackageList(meta->name(), depend.installer(), depend.packages());
            }
        }
    }
}
