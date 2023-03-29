/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef PLUGINSSETTING_H
#define PLUGINSSETTING_H

#include "framework/lifecycle/pluginmetaobject.h"
#include "framework/framework_global.h"

#include <QCoreApplication>
#include <QSettings>

DPF_BEGIN_NAMESPACE

const char ORGANIZATION[] = "deepin";
const char ENABLED[] = "enabled";
const char DISABLED_PLUGINS[] = "Plugins/Disabled";
const char ENABLED_PLUGINS[] = "Plugins/Enabled";

class PluginSetting final : public QSettings
{
    Q_OBJECT
public:
    PluginSetting(Scope scope = Scope::UserScope,
                      const QString &organization = ORGANIZATION,
                      const QString &application = QCoreApplication::applicationName(),
                      QObject *parent = nullptr);
    void setPluginEnable(const PluginMetaObject &meta, bool enabled);
Q_SIGNALS:
    void pluginEnabled(const PluginMetaObject &meta, bool enabled);
};

DPF_END_NAMESPACE

#endif // PLUGINSSETTING_H
