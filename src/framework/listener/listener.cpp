// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listener.h"
#include "framework/listener/private/listener_p.h"
#include "framework/service/qtclassmanager.h"

DPF_BEGIN_NAMESPACE

// 饿汉避免释放冲突
static dpf::Listener listener;

Listener::Listener(QObject *parent)
    : QObject(parent)
    , d(new ListenerPrivate(this))
{

}

Listener &Listener::instance()
{
    return listener;
}

ListenerPrivate::ListenerPrivate(Listener *parent)
    : QObject(parent)
    , q(parent)
{
    QObject::connect(this, &ListenerPrivate::pluginsInitialized,
                     q, &Listener::pluginsInitialized,
                     Qt::UniqueConnection);

    QObject::connect(this, &ListenerPrivate::pluginsStarted,
                     q, &Listener::pluginsStarted,
                     Qt::UniqueConnection);

    QObject::connect(this, &ListenerPrivate::pluginsStoped,
                     q, &Listener::pluginsStoped,
                     Qt::UniqueConnection);
}

DPF_END_NAMESPACE
