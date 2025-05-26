// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINSERVICECONTEXT_H
#define PLUGINSERVICECONTEXT_H

#include "framework/framework_global.h"
#include "framework/service/qtclassfactory.h"
#include "framework/service/qtclassmanager.h"
#include "framework/service/pluginservice.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class DPF_EXPORT PluginServiceContext final : public QObject,
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
class DPF_EXPORT AutoServiceRegister
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
    QString errStr;
    if (!dpf::PluginServiceContext::instance().regClass<T>(T::name(), &errStr)) {
        qCritical() << errStr;
        return false;
    }
    return true;
}

DPF_END_NAMESPACE

#endif // PLUGINSERVICECONTEXT_H
