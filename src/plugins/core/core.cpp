// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core.h"
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

    Controller::instance();

    QObject::connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        ActionManager::getInstance()->readUserSetting();
        uiController.doSwitch(MWNA_RECENT);
    });

    return true;
}

dpf::Plugin::ShutdownFlag Core::stop()
{
    delete Controller::instance();
    return Sync;
}
