// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "framework/framework_global.h"
#include "framework/lifecycle/lifecycle.h"
#include "framework/listener/listener.h"
#include "framework/event/eventcallproxy.h"
#include "framework/service/pluginservicecontext.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class FrameworkPrivate;


/*!
 * \brief The Framework class
 */
class DPF_EXPORT Framework
{
    Q_DISABLE_COPY(Framework)
public:
    static Framework &instance();

    bool initialize();
    bool start();
    const LifeCycle &lifeCycle() const;
    PluginServiceContext &serviceContext() const;
    EventCallProxy &eventProxy() const;
    void enbaleBackTrace() const;
private:
    Framework();

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d), Framework)
    QScopedPointer<FrameworkPrivate> d;
};


DPF_END_NAMESPACE

#define dpfInstance ::dpf::Framework::instance()
#define dpfContext dpfInstance.serviceContext()
#define dpfGetService(T) dpfInstance.serviceContext().service<T>(T::name())

#endif // FRAMEWORK_H
