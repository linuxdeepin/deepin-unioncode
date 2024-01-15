// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"
#include "mainframe/windowkeeper.h"
#include "mainframe/controller.h"
#include "services/window/windowservice.h"

#include <framework/framework.h>
#include "common/common.h"

using namespace dpfservice;

void Core::initialize()
{
    qInfo() << Q_FUNC_INFO;
    QString errStr;

    // 发布窗口服务
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(WindowService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool Core::start()
{
    qInfo() << "set Application Theme";
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        using namespace std::placeholders;

        if (!windowService->addOpenProjectAction) {
            windowService->addOpenProjectAction = std::bind(&WindowKeeper::addOpenProjectAction,
                                                            WindowKeeper::instace(), _1, _2);
        }
    }

    WindowKeeper::instace();

    //Controller::instance();

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        ActionManager::getInstance()->readUserSetting();
        navigation.doSwitch(MWNA_RECENT);
    });

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    delete WindowKeeper::instace();
    return Sync;
}
