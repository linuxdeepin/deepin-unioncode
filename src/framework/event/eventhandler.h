// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "framework/event/event.h"
#include "framework/log/frameworklog.h"
#include "framework/log/codetimecheck.h"
#include "framework/framework_global.h"

#include <QObject>
#include <QtConcurrent>

DPF_BEGIN_NAMESPACE

class Event;
class EventHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EventHandler)

public:
    /*!
     * \brief You should derived the class, and
     * write a static function named type that returns `Type` and `topics`
     *
     * // for example:
     * static Type type()
     * {
     *     return EventHandler::Sync;
     * }
     *
     * static QStringList topics()
     * {
     *     return QStringList() << "TEST_TOPIC";
     * }
     *
     */
    enum class Type : uint8_t
    {
        Sync,
        Async
    };

    explicit EventHandler(QObject *parent = nullptr);
    virtual ~EventHandler() {}

    /*!
     * \brief eventProcess 事件处理入口
     */
    virtual void eventProcess(const Event&) = 0;

    // TODO:
Q_SIGNALS:
    void handError(const QString &error);
    void handInfo(const QString &info);
};

DPF_END_NAMESPACE

#endif // EVENTHANDLER_H
