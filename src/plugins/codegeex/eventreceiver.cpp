// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "common/common.h"
#include "copilot.h"
#include "codegeexmanager.h"
#include "services/project/projectservice.h"

#include <QMenu>

EventReceiverDemo::EventReceiverDemo(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<EventReceiverDemo>()
{
    using namespace std::placeholders;
    eventHandleMap.insert(editor.contextMenu.name, std::bind(&EventReceiverDemo::processContextMenuEvent, this, _1));
    eventHandleMap.insert(editor.textChanged.name, std::bind(&EventReceiverDemo::processTextChangedEvent, this, _1));
    eventHandleMap.insert(editor.selectionChanged.name, std::bind(&EventReceiverDemo::processSelectionChangedEvent, this, _1));
    eventHandleMap.insert(notifyManager.actionInvoked.name, std::bind(&EventReceiverDemo::processActionInvokedEvent, this, _1));
    eventHandleMap.insert(project.openProject.name, std::bind(&EventReceiverDemo::processOpenProjectEvent, this, _1));
}

dpf::EventHandler::Type EventReceiverDemo::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList EventReceiverDemo::topics()
{
    return { T_MENU, editor.topic, notifyManager.topic, project.topic };
}

void EventReceiverDemo::eventProcess(const dpf::Event &event)
{
    const auto &eventName = event.data().toString();
    if (!eventHandleMap.contains(eventName))
        return;

    eventHandleMap[eventName](event);
}

void EventReceiverDemo::processContextMenuEvent(const dpf::Event &event)
{
    QMenu *contextMenu = event.property("menu").value<QMenu *>();
    if (!contextMenu)
        return;

    QMetaObject::invokeMethod(this, [contextMenu]() {
        contextMenu->addMenu(Copilot::instance()->getMenu());
    });
}

void EventReceiverDemo::processTextChangedEvent(const dpf::Event &event)
{
    Copilot::instance()->handleTextChanged();
}

void EventReceiverDemo::processSelectionChangedEvent(const dpf::Event &event)
{
    QString fileName = event.property("fileName").toString();
    int lineFrom = event.property("lineFrom").toInt();
    int indexFrom = event.property("indexFrom").toInt();
    int lineTo = event.property("lineTo").toInt();
    int indexTo = event.property("indexTo").toInt();
    Copilot::instance()->handleSelectionChanged(fileName, lineFrom, indexFrom, lineTo, indexTo);
}

void EventReceiverDemo::processActionInvokedEvent(const dpf::Event &event)
{
    auto actId = event.property("actionId").toString();
    if (actId == "codegeex_login_default")
        QMetaObject::invokeMethod(CodeGeeXManager::instance(), "login", Qt::QueuedConnection);
    else if (actId == "ai_rag_install")
        CodeGeeXManager::instance()->installConda();
}

void EventReceiverDemo::processOpenProjectEvent(const dpf::Event &event)
{
    auto projectPath = event.property("workspace").toString();
    QJsonObject results = CodeGeeXManager::instance()->query(projectPath, "", 1);
    if (results["chunks"].toArray().size() != 0) // project has generated, update it
        CodeGeeXManager::instance()->generateRag(projectPath);
}
