// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"

#include "common/util/eventdefinitions.h"

UTEventReceiver::UTEventReceiver(QObject *parent)
    : dpf::EventHandler(parent),
      dpf::AutoEventHandlerRegister<UTEventReceiver>()
{
    using namespace std::placeholders;
    eventHandleMap.insert(ai.LLMChanged.name, std::bind(&UTEventReceiver::processLLMCountChanged, this, _1));
}

dpf::EventHandler::Type UTEventReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList UTEventReceiver::topics()
{
    return { ai.topic };
}

void UTEventReceiver::eventProcess(const dpf::Event &event)
{
    const auto &eventName = event.data().toString();
    if (!eventHandleMap.contains(eventName))
        return;

    eventHandleMap[eventName](event);
}

void UTEventReceiver::processLLMCountChanged(const dpf::Event &event)
{
    Q_EMIT EventDistributeProxy::instance()->sigLLMCountChanged();
}

EventDistributeProxy *EventDistributeProxy::instance()
{
    static EventDistributeProxy ins;
    return &ins;
}
