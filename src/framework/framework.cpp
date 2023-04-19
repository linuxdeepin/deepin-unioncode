/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
    QScopedPointer<AppBus> appBus;
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
#ifndef NO_BACKTRACE
    backtrace::initbacktrace();
#endif
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

AppBus &Framework::appBus() const
{
    if (!d->appBus) {
        d->appBus.reset(new AppBus());
    }
    return *d->appBus;
}

Framework::Framework() :
    d(new FrameworkPrivate(this))
{
    d->lifeCycle.reset(new LifeCycle());
}

DPF_END_NAMESPACE
