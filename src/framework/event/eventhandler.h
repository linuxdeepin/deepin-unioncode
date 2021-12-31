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
