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
#include "eventcallproxy.h"

DPF_BEGIN_NAMESPACE

EventCallProxy &EventCallProxy::instance()
{
    static EventCallProxy proxy;
    return proxy;
}

/*!
 * \brief EventCallProxy::pubEvent Send event objects to
 * all `handlers` that have subscribed to the topic，
 * and the `eventProcess` function of the `handler` will be called
 * \param event
 * \return true if success
 */
bool EventCallProxy::pubEvent(const Event &event)
{
    bool flag = false;
    QList<EventCallProxy::HandlerInfo> &infoGroup = getInfoList();
    for (HandlerInfo &info : infoGroup) {
        if (!info.topics.contains(event.topic()))
            continue;
        if (Q_LIKELY(info.invoke)) {
            info.invoke(info, event);
            flag = true;
        }
    }
    return flag;
}

/*!
 * \brief EventCallProxy::registerHandler no need for developers to call directly，
 *  classes that inherit from `AutoEventHandlerRegister` will automatically call
 * \param type
 * \param topics
 * \param creator
 */

void EventCallProxy::registerHandler(EventHandler::Type type, const QStringList &topics, EventCallProxy::CreateFunc creator)
{
    QMutexLocker locker(eventMutex());
    auto &infoList = getInfoList();
    ExportFunc invoke {nullptr};
    if (type == EventHandler::Type::Sync) {
        invoke = [creator] (HandlerInfo &info, const Event &event) {
            fillInfo(info, creator);
            info.handler->eventProcess(event);
        };
    }

    if (type ==  EventHandler::Type::Async) {
        invoke = [creator] (HandlerInfo &info, const Event &event) {
            fillInfo(info, creator);
            info.future = QtConcurrent::run(info.handler.data(), &EventHandler::eventProcess, event);
        };
    }
    qDebug() << "Register Handler, type " << static_cast<int>(type) << ", topics" << topics;
    infoList.append(HandlerInfo{nullptr, invoke, topics, QFuture<void>()});
}


QList<EventCallProxy::HandlerInfo> &EventCallProxy::getInfoList()
{
    static QList<HandlerInfo> eventHandlers;
    return eventHandlers;
}

void EventCallProxy::fillInfo(EventCallProxy::HandlerInfo &info, EventCallProxy::CreateFunc creator)
{
    if (!info.handler)
        info.handler = creator();
}

QMutex *EventCallProxy::eventMutex()
{
    static QMutex mutex;
    return &mutex;
}

DPF_END_NAMESPACE






