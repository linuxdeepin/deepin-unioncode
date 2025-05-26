// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "templateplugin.h"
#include "templatemanager.h"

void TemplatePlugin::initialize()
{
    qInfo() << __FUNCTION__;
}

bool TemplatePlugin::start()
{
    qInfo() << __FUNCTION__;
    TemplateManager::instance()->initialize();

    return true;
}

dpf::Plugin::ShutdownFlag TemplatePlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


