// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "common/common.h"
#include "debuggersignals.h"
#include "debuggerglobals.h"

DebugEventReceiver::DebugEventReceiver(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<DebugEventReceiver>()
{
}

dpf::EventHandler::Type DebugEventReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList DebugEventReceiver::topics()
{
    return { T_BUILDER, project.topic, debugger.topic, editor.topic };
}

void DebugEventReceiver::eventProcess(const dpf::Event &event)
{
    emit debuggerSignals->receivedEvent(event);
}

SyncDebugEventReceiver::SyncDebugEventReceiver(QObject *parent)
    : dpf::EventHandler(parent),
      dpf::AutoEventHandlerRegister<SyncDebugEventReceiver>()
{
}

dpf::EventHandler::Type SyncDebugEventReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList SyncDebugEventReceiver::topics()
{
    return { session.topic };
}

void SyncDebugEventReceiver::eventProcess(const dpf::Event &event)
{
    emit debuggerSignals->receivedEvent(event);
}
