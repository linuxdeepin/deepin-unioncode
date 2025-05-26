// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTENER_P_H
#define LISTENER_P_H

#include "framework/framework_global.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class Listener;
class ListenerPrivate final : public QObject
{
    Q_OBJECT
    friend class Listener;
    Listener *const q;
    friend class PluginManagerPrivate;
public:
    explicit ListenerPrivate(Listener *parent);
signals:
    void pluginsInitialized();
    void pluginsStarted();
    void pluginsStoped();
};

DPF_END_NAMESPACE

#endif // LISTENER_P_H
