// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
class DPF_EXPORT Event final
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
    typedef QVariant Arg;
    typedef const QVariant cArg;
    typedef QVector<QVariant> Args;
    EventInterface() = delete;

    EventInterface(const QString &name, const QVector<QString> &keys,
                   const std::function<void(const QVector<QVariant>&)> &func)
        : std::function<void (const QVector<QVariant> &)> (func), name(name), pKeys(keys) {}

    void operator()(const Args &as) const {
        return std::function<void(const QVector<QVariant> &)>::operator()(as);
    }

    void operator()() const { operator()(Args{}); }

    template<class T0>
    void operator()(const T0 &v0) const { operator()(Args{ Arg::fromValue<T0>(v0) }); }

    template<class T0, class T1>
    void operator()(const T0 &v0, const T1 &v1) const {
        operator()(Args{ Arg::fromValue<T0>(v0), Arg::fromValue<T1>(v1) });
    }

    template<class T0, class T1, class T2>
    void operator()(const T0 &v0, const T1 &v1, const T2 &v2) const {
        operator()(Args{
            Arg::fromValue<T0>(v0), Arg::fromValue<T1>(v1),
                    Arg::fromValue<T2>(v2)
        });
    }

    template<class T0, class T1, class T2, class T3>
    void operator()(const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3) const {
        operator()(Args{
            Arg::fromValue<T0>(v0), Arg::fromValue<T1>(v1),
                    Arg::fromValue<T2>(v2), Arg::fromValue<T3>(v3)
        });
    }

    template<class T0, class T1, class T2, class T3, class T4>
    void operator()(const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4) const {
        operator()(Args{
            Arg::fromValue<T0>(v0), Arg::fromValue<T1>(v1),
                    Arg::fromValue<T2>(v2), Arg::fromValue<T3>(v3),
                    Arg::fromValue<T4>(v4)
        });
    }

    template<class T0, class T1, class T2, class T3, class T4, class T5>
    void operator()(const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4,
                    const T5 &v5) const {
        operator()(Args{
            Arg::fromValue<T0>(v0), Arg::fromValue<T1>(v1),
                    Arg::fromValue<T2>(v2), Arg::fromValue<T3>(v3),
                    Arg::fromValue<T4>(v4), Arg::fromValue<T5>(v5)
        });
    }

    template<class T0, class T1, class T2, class T3, class T4, class T5, class T6>
    void operator()(const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4,
                    const T5 &v5, const T6 &v6) const {
        operator()(Args{
            Arg::fromValue<T0>(v0), Arg::fromValue<T1>(v1),
                    Arg::fromValue<T2>(v2), Arg::fromValue<T3>(v3),
                    Arg::fromValue<T4>(v4), Arg::fromValue<T5>(v5),
                    Arg::fromValue<T6>(v6)
        });
    }

    template<class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
    void operator()(const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4,
                    const T5 &v5, const T6 &v6, const T7 &v7) const {
        operator()(Args{
            Arg::fromValue<T0>(v0), Arg::fromValue<T1>(v1),
                    Arg::fromValue<T2>(v2), Arg::fromValue<T3>(v3),
                    Arg::fromValue<T4>(v4), Arg::fromValue<T5>(v5),
                    Arg::fromValue<T6>(v6), Arg::fromValue<T7>(v7)
        });
    }

    template<class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
    void operator()(const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4,
                    const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8) const{
        operator()(Args{
            Arg::fromValue<T0>(v0), Arg::fromValue<T1>(v1),
                    Arg::fromValue<T2>(v2), Arg::fromValue<T3>(v3),
                    Arg::fromValue<T4>(v4), Arg::fromValue<T5>(v5),
                    Arg::fromValue<T6>(v6), Arg::fromValue<T7>(v7),
                    Arg::fromValue<T8>(v8),
        });
    }

    template<class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
    void operator()(const T0 &v0, const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4,
                    const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8, const T9 &v9) const {
        operator()(Args{
            Arg::fromValue<T0>(v0), Arg::fromValue<T1>(v1),
                    Arg::fromValue<T2>(v2), Arg::fromValue<T3>(v3),
                    Arg::fromValue<T4>(v4), Arg::fromValue<T5>(v5),
                    Arg::fromValue<T6>(v6), Arg::fromValue<T7>(v7),
                    Arg::fromValue<T8>(v8), Arg::fromValue<T9>(v9)
        });
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
#define OPI_OBJECT(t, logics) inline const struct { const char* topic{#t} ; logics }t;
#define OPI_INTERFACE(d, ...) const dpf::EventInterface d { #d, {__VA_ARGS__} , [=](const QVector<QVariant> &args) -> void {\
    OPI_ASKEEP(d.pKeys, args);\
    dpf::Event event(topic); event.setData(#d);\
    for ( int idx = 0; idx < d.pKeys.size(); idx ++) { event.setProperty(d.pKeys[idx], args[idx]); }\
    dpf::EventCallProxy::instance().pubEvent(event);\
    }};

DPF_END_NAMESPACE

#endif // EVENT_H
