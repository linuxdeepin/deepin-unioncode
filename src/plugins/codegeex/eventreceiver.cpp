// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "common/common.h"
#include "copilot.h"
#include "codegeexmanager.h"

#include <QMenu>

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
    return { T_MENU, editor.topic, notifyManager.topic, project.topic };
}

void EventReceiverDemo::eventProcess(const dpf::Event &event)
{
    if (event.topic() == editor.topic) {
        QString eventData = event.data().toString();
        if (eventData == "contextMenu") {
            QMenu *contextMenu = event.property("menu").value<QMenu *>();
            if (!contextMenu)
                return;

            QMetaObject::invokeMethod(this, [contextMenu]() {
                contextMenu->addMenu(Copilot::instance()->getMenu());
            });
        } else if (eventData == "textChanged") {
            Copilot::instance()->handleTextChanged();
        }
    } else if (event.topic() == notifyManager.topic) {
        QString eventData = event.data().toString();
        if (eventData == "actionInvoked") {
            auto actId = event.property("actionId").toString();
            if (actId == "codegeex_login_default")
                QMetaObject::invokeMethod(CodeGeeXManager::instance(), "login", Qt::QueuedConnection);
            else if (actId == "ai_rag_install")
                CodeGeeXManager::instance()->installConda();
        }
    } else if (event.topic() == project.topic) {
        QString eventData = event.data().toString();
        if (eventData == "openProject") {
            auto projectPath = event.property("workspace").toString();
            CodeGeeXManager::instance()->generateRag(projectPath);
        }
    }
}
