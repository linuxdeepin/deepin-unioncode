// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "framework.h"
#include "backtrace/backtrace.h"
#include "framework/log/frameworklog.h"
#include "framework/lifecycle/lifecycle.h"

DPF_BEGIN_NAMESPACE

class FrameworkPrivate
{
    friend class Framework;
    Framework *const q;
    // Plugin lifeCycle manager.
    QScopedPointer<LifeCycle> lifeCycle;
    bool bInitialized = false;

    explicit FrameworkPrivate(Framework *dd);
};

FrameworkPrivate::FrameworkPrivate(Framework *dd)
    : q(dd)
{

}

/*!
 * \brief Get framework instance.
 * \return
 */
Framework &Framework::instance()
{
    static Framework ins;
    return ins;
}


/*!
 * \brief Framework inner modules will be initialized
 * when it invoked,same for plugins.
 * \return
 */
bool Framework::initialize()
{
    if (d->bInitialized) {
        qDebug() << "Frame work has been initialized!";
        return true;
    }
    FrameworkLog::initialize();

    // It will be true after all inner moudules initialized
    // successfully.
    d->bInitialized = true;

    return true;
}


/*!
 * \brief Start framework after initialized.
 * \return
 */
bool Framework::start()
{
    // TODO(anyone):Start plugin after initialized,
    // thus plugin logic will be run
    return true;
}

/*!
 * \brief Get plugin life cycle manager
 * \return
 */
const LifeCycle &Framework::lifeCycle() const
{
    return *d->lifeCycle;
}

/*!
 * \brief Get plugin service context
 * \return
 */
PluginServiceContext &Framework::serviceContext() const
{
    return PluginServiceContext::instance();
}

/*!
 * \brief Get event proxy
 * \return
 */
EventCallProxy &Framework::eventProxy() const
{
    return EventCallProxy::instance();
}

void Framework::enbaleBackTrace() const
{
    backtrace::initbacktrace();
}

Framework::Framework() :
    d(new FrameworkPrivate(this))
{
    d->lifeCycle.reset(new LifeCycle());
}

DPF_END_NAMESPACE
