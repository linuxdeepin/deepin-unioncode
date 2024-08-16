// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "git.h"
#include "constants.h"
#include "client/gitclient.h"
#include "utils/gitmenumanager.h"

#include "services/editor/editorservice.h"
#include "services/window/windowservice.h"

using namespace dpfservice;

void Git::initialize()
{
}

bool Git::start()
{
    GitClient::instance()->init();

    initWindowService();
    connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        auto editSrv = dpfGetService(EditorService);
        if (!editSrv)
            return;

        auto widget = qobject_cast<AbstractEditWidget *>(GitClient::instance()->gitTabWidget());
        if (widget)
            editSrv->registerWidget(GitWindow, widget);
    });

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

    GitMenuManager::instance()->initialize();
    windowService->addStatusBarItem(GitClient::instance()->instantBlameWidget());
}
