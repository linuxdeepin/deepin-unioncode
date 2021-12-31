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
#ifndef PLUGINDEPEND_H
#define PLUGINDEPEND_H

#include "framework/framework_global.h"

#include <QString>

DPF_BEGIN_NAMESPACE

/**
 * @brief The PluginDepend class
 * 插件依赖项类
 */
class PluginDepend final
{
    friend class PluginManager;
    friend class PluginManagerPrivate;
    friend class PluginMetaObject;
    friend Q_CORE_EXPORT QDebug operator<< (QDebug, const PluginDepend &);

    QString pluginName;
    QString pluginVersion;

public:
    PluginDepend(const PluginDepend &depend);
    PluginDepend& operator = (const PluginDepend &depend);
    QString name() const {return pluginName;}
    QString version() const {return pluginVersion;}

private:
    PluginDepend();
};

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<< (QDebug, const DPF_NAMESPACE::PluginDepend &);
#endif //QT_NO_DEBUG_STREAM
QT_END_NAMESPACE

DPF_END_NAMESPACE

#endif // PLUGINDEPEND_H
