// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENT_P_H
#define EVENT_P_H

#include "framework/event/event.h"

#include <QString>
#include <QVariant>

DPF_BEGIN_NAMESPACE

class EventPrivate
{
    friend class Event;
    Event *const q;
    QHash<QString,QVariant> sourceHash;
    friend Q_CORE_EXPORT QDebug operator <<(QDebug, const Event &);
public:

    explicit EventPrivate(Event *qq)
        : q(qq)
    {

    }
};

DPF_END_NAMESPACE

#endif // EVENT_P_H
