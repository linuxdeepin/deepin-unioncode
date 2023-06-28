// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandproxy.h"

void CommandProxy::initialize()
{
    qInfo() << __FUNCTION__;
}

bool CommandProxy::start()
{
    qInfo() << __FUNCTION__;
    return true;
}

dpf::Plugin::ShutdownFlag CommandProxy::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}
