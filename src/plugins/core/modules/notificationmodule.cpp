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
    auto notify = qOverload<uint, const QString &, const QString &, const QStringList &>(&NotificationManager::notify);
    windowService->notify = std::bind(notify, NotificationManager::instance(), _1, _2, _3, _4);

    auto notifyWithCb = qOverload<uint, const QString &, const QString &, const QStringList &, NotifyCallback>(&NotificationManager::notify);
    windowService->notifyWithCallback = std::bind(notifyWithCb, NotificationManager::instance(), _1, _2, _3, _4, _5);
}
