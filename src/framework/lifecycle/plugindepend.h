// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    QString toString() const {return pluginName + " (" + pluginVersion + ")";}

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
