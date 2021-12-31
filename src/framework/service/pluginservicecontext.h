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
#ifndef PLUGINSERVICECONTEXT_H
#define PLUGINSERVICECONTEXT_H

#include "framework/framework_global.h"
#include "framework/service/qtclassfactory.h"
#include "framework/service/qtclassmanager.h"
#include "framework/service/pluginservice.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class PluginServiceContext final : public QObject,
        public QtClassFactory<PluginService>,
        public QtClassManager<PluginService>
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginServiceContext)

public:
    static PluginServiceContext &instance();
    QStringList services();
    bool load(const QString &name, QString *errString = nullptr);
    bool unload(const QString &name);

    template<class CT = PluginService>
    CT *service(const QString &name)
    {
        return qobject_cast<CT*>(PluginServiceContext::instance().
                                 value(name));
    }

private:
    explicit PluginServiceContext(QObject *parent = nullptr): QObject (parent){}
};


// auto register all services
template <typename T>
class AutoServiceRegister
{
public:
    AutoServiceRegister()
    {
        // must keep it!!!
        // Otherwise `trigger` will not be called !
        qDebug() << isRegistered;
    }

    static bool trigger();

private:
    static bool isRegistered;
};

template <typename T>
bool AutoServiceRegister<T>::isRegistered = AutoServiceRegister<T>::trigger();
template <typename T>
bool AutoServiceRegister<T>::trigger()
{
    qInfo() << "Register Service: " << __PRETTY_FUNCTION__;
    QString errStr;
    if (!dpf::PluginServiceContext::instance().regClass<T>(T::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
    return true;
}

DPF_END_NAMESPACE

#endif // PLUGINSERVICECONTEXT_H
