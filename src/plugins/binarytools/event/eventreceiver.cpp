// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "constants.h"
#include "configure/binarytoolsmanager.h"

#include "common/util/eventdefinitions.h"

EventReceiver::EventReceiver(QObject *parent)
{
    using namespace std::placeholders;
    eventHandleMap.insert(notifyManager.actionInvoked.name, std::bind(&EventReceiver::handleNotifyEvent, this, _1));
    eventHandleMap.insert(editor.fileSaved.name, std::bind(&EventReceiver::handleFileSavedEvent, this, _1));
}

dpf::EventHandler::Type EventReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList EventReceiver::topics()
{
    return { editor.topic, notifyManager.topic };
}

void EventReceiver::eventProcess(const dpf::Event &event)
{
    const auto &eventName = event.data().toString();
    if (!eventHandleMap.contains(eventName))
        return;

    eventHandleMap[eventName](event);
}

void EventReceiver::handleNotifyEvent(const dpf::Event &event)
{
    const auto &actId = event.property("actionId").toString();
    auto toolId = actId.mid(0, actId.indexOf('_'));
    BinaryToolsManager::instance()->installTool(toolId);
}

void EventReceiver::handleFileSavedEvent(const dpf::Event &event)
{
    const auto &fileName = event.property("fileName").toString();
    BinaryToolsManager::instance()->eventTriggered(DocumentSaveEvent, QVariantList() << fileName);
}
