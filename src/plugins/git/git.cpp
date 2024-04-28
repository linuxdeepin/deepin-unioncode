// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "git.h"
#include "constants.h"
#include "client/gitclient.h"
#include "utils/gitmenumanager.h"

#include "base/abstractwidget.h"
#include "services/window/windowservice.h"

using namespace dpfservice;

void Git::initialize()
{
}

bool Git::start()
{
    GitClient::instance()->init();

    initWindowService();

    return true;
}

dpf::Plugin::ShutdownFlag Git::stop()
{
    return Sync;
}

void Git::initWindowService()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    GitMenuManager::instance()->initialize(windowService);
    windowService->addStatusBarItem(GitClient::instance()->instantBlameWidget());
    windowService->registerWidget(GitWindow, new AbstractWidget(GitClient::instance()->gitTabWidget()));
}
