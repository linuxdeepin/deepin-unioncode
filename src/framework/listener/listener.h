// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTENER_H
#define LISTENER_H

#include "framework/framework_global.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class ListenerPrivate;
class DPF_EXPORT Listener final : public QObject
{
    Q_OBJECT
    friend class ListenerPrivate;
    ListenerPrivate *const d;
    friend class PluginManagerPrivate;
public:
    explicit Listener(QObject *parent = nullptr);
    static Listener &instance();
signals:
    void pluginsInitialized();
    void pluginsStarted();
    void pluginsStoped();
};

DPF_END_NAMESPACE

#endif // LISTENER_H
