/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
