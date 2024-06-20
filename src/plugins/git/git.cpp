// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "git.h"
#include "client/gitclient.h"

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

    windowService->addStatusBarItem(GitClient::instance()->instantBlameWidget());
}
