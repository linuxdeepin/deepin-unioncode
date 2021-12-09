/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef EVENTCALLPROXY_H
#define EVENTCALLPROXY_H

#include "framework/event/event.h"
#include "framework/event/eventhandler.h"
#include "framework/log/frameworklog.h"

#include <QObject>
#include <QMutex>
#include <QDebug>

#include <functional>
#include <memory>


DPF_BEGIN_NAMESPACE
class EventCallProxy final
{
    template <typename T>
    friend class AutoEventHandlerRegister;
    struct HandlerInfo;
    using CreateFunc = std::function<QSharedPointer<EventHandler>()> ;
    using ExportFunc = std::function<void(HandlerInfo &info, const Event &event)>;

    struct HandlerInfo
    {
        QSharedPointer<EventHandler> handler;
        ExportFunc invoke;
        QStringList topics;
        QFuture<void> future;
    };

public:
    static EventCallProxy &instance();
    bool pubEvent(const Event& event);

private:
    static void registerHandler(EventHandler::Type type, const QStringList &topics, CreateFunc creator);
    static QList<HandlerInfo> &getInfoList();
    static void fillInfo(HandlerInfo &info, CreateFunc creator);
    static QMutex *eventMutex();
};

// auto register all event handler
template <typename T>
class AutoEventHandlerRegister
{
public:
    AutoEventHandlerRegister()
    {
        // must keep it!!!
        // Otherwise `trigger` will not be called !
        qDebug() << isRegistered;
    }

    static bool trigger();

private:
    static bool isRegistered;
};

// for example:

/*!
 * class WindowEventHandler: public EventHandler, AutoEventHandlerRegister<WindowEventHandler>
 * {
 *     Q_OBJECT
 *
 * public:
 *     WindowEventHandler(): AutoEventHandlerRegister<WindowEventHandler>() {}
 *
 *     static EventHandler::Type type()
 *     {
 *         return EventHandler::Type::Sync;
 *     }
 *
 *     static QStringList topics()
 *     {
 *          return QStringList() << "WindowEvent";
 *     }
 * };
 */


template <typename T>
bool AutoEventHandlerRegister<T>::isRegistered = AutoEventHandlerRegister<T>::trigger();
template <typename T>
bool AutoEventHandlerRegister<T>::trigger()
{
    qInfo() << "Register: " << __PRETTY_FUNCTION__;
    EventCallProxy::registerHandler(T::type(), T::topics(), [] { return QSharedPointer<T>(new T()); });
    return true;
}

DPF_END_NAMESPACE

#endif // EVENTCALLPROXY_H
