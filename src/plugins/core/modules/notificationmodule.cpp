// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationmodule.h"
#include "notify/notificationmanager.h"

#include "services/window/windowservice.h"

using namespace dpfservice;

void NotificationModule::initialize(Controller *_uiController)
{
    Q_UNUSED(_uiController)

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    Q_ASSERT(windowService);

    using namespace std::placeholders;
    windowService->notify = std::bind(&NotificationManager::notify, NotificationManager::instance(), _1, _2, _3, _4);
}
