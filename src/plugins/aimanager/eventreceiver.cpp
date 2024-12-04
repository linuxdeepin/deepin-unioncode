// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "common/common.h"

AiManagerReceiver::AiManagerReceiver(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<AiManagerReceiver>()
{
}

dpf::EventHandler::Type AiManagerReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList AiManagerReceiver::topics()
{
    return {};
}

void AiManagerReceiver::eventProcess(const dpf::Event &event)
{
    QString data = event.data().toString();
}
