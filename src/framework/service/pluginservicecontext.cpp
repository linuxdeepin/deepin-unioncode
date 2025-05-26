// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginservicecontext.h"

DPF_BEGIN_NAMESPACE

PluginServiceContext &PluginServiceContext::instance()
{
    static PluginServiceContext ctx;
    return ctx;
}

QStringList PluginServiceContext::services()
{
    return PluginServiceContext::instance().keys();
}

bool PluginServiceContext::load(const QString &name, QString *errString)
{
    auto ins = this->create(name, errString);
    if (!ins)
        return false;
    if (!this->append(name, ins, errString))
        return false;
    return true;
}

bool PluginServiceContext::unload(const QString &name)
{
    return this->remove(name);
}

DPF_END_NAMESPACE
