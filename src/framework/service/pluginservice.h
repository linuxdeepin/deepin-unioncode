// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINSERVICE_H
#define PLUGINSERVICE_H

#include "framework/log/frameworklog.h"
#include "framework/framework_global.h"

#include <QObject>
#include <QHash>

#include <functional>

#define DPF_INTERFACE(ret, method, ...) std::function<ret(__VA_ARGS__)> method{nullptr}

DPF_BEGIN_NAMESPACE

class PluginService : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginService)
public:
    explicit PluginService(QObject *parent = nullptr): QObject(parent) {}
    virtual ~PluginService(){}
};

DPF_END_NAMESPACE

#endif // PLUGINSERVICE_H
