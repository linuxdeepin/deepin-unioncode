// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"
#include "uicontroller//controller.h"
#include "services/window/windowservice.h"
#include "services/session/sessionservice.h"
#include "locator/locatormanager.h"
#include "locator/actionlocator.h"

#include <framework/framework.h>
#include "common/common.h"

using namespace dpfservice;

void Core::initialize()
{
    QString errStr;

    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(WindowService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
    if (!ctx.load(SessionService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool Core::start()
{
    Controller::instance();

    auto actionLocator = new ActionLocator(this);
    LocatorManager::instance()->registerLocator(actionLocator);

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        uiController.doSwitch(MWNA_RECENT);
    });

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    return Sync;
}
