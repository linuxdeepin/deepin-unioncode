// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "common/common.h"

EventReceiverDemo::EventReceiverDemo(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<EventReceiverDemo>()
{
}

dpf::EventHandler::Type EventReceiverDemo::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList EventReceiverDemo::topics()
{
    return {T_MENU};
}

void EventReceiverDemo::eventProcess(const dpf::Event &event)
{
}


