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
#ifndef EVENT_H
#define EVENT_H

#include "framework/framework_global.h"

#include <QString>
#include <QVector>
#include <QVariant>
#include <QSharedData>

DPF_BEGIN_NAMESPACE

class EventPrivate;

/**
 * @brief The Event class
 *  事件数据源，只能当做类使用不可继承
 *  禁止被继承
 */
class Event final
{
    EventPrivate *d;
    friend Q_CORE_EXPORT QDebug operator <<(QDebug, const Event &);

public:
    Event();
    explicit Event(const QString &topic);
    explicit Event(const Event& event);
    ~Event();
    Event &operator =(const Event &);

    void setTopic(const QString &topic);
    QString topic() const;

    void setData(const QVariant &data);
    QVariant data() const;

    void setProperty(const QString& key, const QVariant value);
    QVariant property(const QString &key) const;
};

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator <<(QDebug, const DPF_NAMESPACE::Event &);
#endif //QT_NO_DEBUG_STREAM
QT_END_NAMESPACE

struct EventInterface : std::function<void(const QVector<QVariant> &)>
{
    QString name;
    QVector<QString> pKeys;
    EventInterface() = delete;

    EventInterface(const QString &name, const QVector<QString> &keys, const std::function<void(const QVector<QVariant>&)> &func)
        : std::function<void (const QVector<QVariant> &)> (func) ,  name(name), pKeys(keys) {}

    void operator()(const QVector<QVariant> &as) const
    {
        return std::function<void(const QVector<QVariant> &)>::operator()(as);
    }
};

/**
 * @brief The OPI_OBJECT macro
 *  Declare and define topic scope
 *
 * @brief The OPI_INTERFACE macro
 *  Declare and define call event interface
 *
 * @details
 * Usage:
 * Call mode:
 *     The interface implemented by the functor will be encapsulated for the time sending interface,
 *     for send event package:
 *         Event {
 *             topic : "collaborators"
 *             data : "openRepos"
 *             property { "workspace": "/usr/home/test" }
 *         };
 * Usage:
 *
 * First: The receiver of eventHandler needs to be implemented in the plugin,
 *        abstract interface file in eventhandler.h/.cpp
 *
 * Second: Decl from send event interface to any file. (If you are cross plug-in event call,
 *         I recommend you define it in the header file that can be referenced by multiple plug-ins.)
 *         @code
 *         OPI_OBJECT(collaborators,
 *                    OPI_INTERFACE(openRepos, "workspace")
 *                    )
 *         @endcode
 *
 * Third: call event:
 *        @code
 *        collaborators.openRepos("/usr/home/test");
 *        @endcode
 *
 */
#define OPI_ASKEEP(pKeys, pVals) if (pKeys.size() != pVals.size()) { qCritical() << "Key value pair length mismatch"; abort(); }
#define OPI_OBJECT(t, logics) inline const struct { const QString topic{#t} ; logics }t;
#define OPI_INTERFACE(d, ...) const EventInterface d { #d, {__VA_ARGS__} , [=](const QVector<QVariant> &args) -> void {\
    OPI_ASKEEP(d.pKeys, args);\
    dpf::Event event(topic); event.setData(#d);\
    for ( int idx = 0; idx < d.pKeys.size(); idx ++) { event.setProperty(d.pKeys[idx], args[idx]); }\
    dpf::EventCallProxy::instance().pubEvent(event);\
    }};

DPF_END_NAMESPACE

#endif // EVENT_H
