// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
class DPF_EXPORT EventCallProxy final
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
    EventCallProxy::registerHandler(T::type(), T::topics(), [] { return QSharedPointer<T>(new T()); });
    return true;
}

DPF_END_NAMESPACE

#endif // EVENTCALLPROXY_H
