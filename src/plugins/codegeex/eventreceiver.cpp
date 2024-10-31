// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "common/common.h"
#include "copilot.h"
#include "codegeexmanager.h"
#include "services/project/projectservice.h"
#include "services/window/windowservice.h"

#include <QMenu>

CodeGeeXReceiver::CodeGeeXReceiver(QObject *parent)
    : dpf::EventHandler(parent), dpf::AutoEventHandlerRegister<CodeGeeXReceiver>()
{
    using namespace std::placeholders;
    eventHandleMap.insert(editor.contextMenu.name, std::bind(&CodeGeeXReceiver::processContextMenuEvent, this, _1));
    eventHandleMap.insert(editor.selectionChanged.name, std::bind(&CodeGeeXReceiver::processSelectionChangedEvent, this, _1));
    eventHandleMap.insert(editor.inlineWidgetClosed.name, std::bind(&CodeGeeXReceiver::processInlineWidgetClosedEvent, this, _1));
    eventHandleMap.insert(notifyManager.actionInvoked.name, std::bind(&CodeGeeXReceiver::processActionInvokedEvent, this, _1));
    eventHandleMap.insert(project.openProject.name, std::bind(&CodeGeeXReceiver::processOpenProjectEvent, this, _1));
    eventHandleMap.insert(uiController.switchToWidget.name, std::bind(&CodeGeeXReceiver::processSwitchToWidget, this, _1));
}

dpf::EventHandler::Type CodeGeeXReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList CodeGeeXReceiver::topics()
{
    return { T_MENU, editor.topic, notifyManager.topic, project.topic, uiController.topic };
}

void CodeGeeXReceiver::eventProcess(const dpf::Event &event)
{
    const auto &eventName = event.data().toString();
    if (!eventHandleMap.contains(eventName))
        return;

    eventHandleMap[eventName](event);
}

void CodeGeeXReceiver::processContextMenuEvent(const dpf::Event &event)
{
    QMenu *contextMenu = event.property("menu").value<QMenu *>();
    if (!contextMenu)
        return;

    contextMenu->addMenu(Copilot::instance()->getMenu());
}

void CodeGeeXReceiver::processSelectionChangedEvent(const dpf::Event &event)
{
    QString fileName = event.property("fileName").toString();
    int lineFrom = event.property("lineFrom").toInt();
    int indexFrom = event.property("indexFrom").toInt();
    int lineTo = event.property("lineTo").toInt();
    int indexTo = event.property("indexTo").toInt();
    Copilot::instance()->handleSelectionChanged(fileName, lineFrom, indexFrom, lineTo, indexTo);
}

void CodeGeeXReceiver::processInlineWidgetClosedEvent(const dpf::Event &event)
{
    Copilot::instance()->handleInlineWidgetClosed();
}

void CodeGeeXReceiver::processActionInvokedEvent(const dpf::Event &event)
{
    auto actId = event.property("actionId").toString();
    if (actId == "codegeex_login_default")
        CodeGeeXManager::instance()->login();
    else if (actId == "ai_rag_install")
        CodeGeeXManager::instance()->installConda();
}

void CodeGeeXReceiver::processOpenProjectEvent(const dpf::Event &event)
{
    QtConcurrent::run([=](){
        auto projectPath = event.property("workspace").toString();
        QJsonObject results = CodeGeeXManager::instance()->query(projectPath, "", 1);
        if (results["Chunks"].toArray().size() != 0)   // project has generated, update it
            CodeGeeXManager::instance()->generateRag(projectPath);
    });
}

void CodeGeeXReceiver::processSwitchToWidget(const dpf::Event &event)
{
    auto widgetName = event.property("name").toString();
    using namespace dpfservice;
    if (widgetName != MWNA_EDIT)
        return;
    auto windowService = dpfGetService(WindowService);
    windowService->showWidgetAtRightspace(MWNA_CODEGEEX);
}

CodeGeeXCallProxy::CodeGeeXCallProxy()
{
}

CodeGeeXCallProxy *CodeGeeXCallProxy::instance()
{
    static CodeGeeXCallProxy proxy;
    return &proxy;
}
