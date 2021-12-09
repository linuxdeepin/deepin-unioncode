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
#ifndef EVENT_H
#define EVENT_H

#include "framework/framework_global.h"

#include <QString>
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

DPF_END_NAMESPACE

#endif // EVENT_H
