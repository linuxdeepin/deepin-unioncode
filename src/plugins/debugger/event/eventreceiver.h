// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include <framework/framework.h>
#include <QObject>

class DebugEventReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<DebugEventReceiver>
{
    friend class dpf::AutoEventHandlerRegister<DebugEventReceiver>;

public:
    explicit DebugEventReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();

private:
    virtual void eventProcess(const dpf::Event &event) override;
};

class SyncDebugEventReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<SyncDebugEventReceiver>
{
    friend class dpf::AutoEventHandlerRegister<SyncDebugEventReceiver>;

public:
    explicit SyncDebugEventReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();

private:
    virtual void eventProcess(const dpf::Event &event) override;
};

#endif   // EVENTRECEIVER_H
