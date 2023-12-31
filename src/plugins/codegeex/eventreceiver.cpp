// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "common/common.h"
#include "copilot.h"

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
    return {T_MENU, editor.topic};
}

void EventReceiverDemo::eventProcess(const dpf::Event &event)
{
    if (event.topic() == editor.topic) {
        QString eventData = event.data().toString();
        if (eventData == "contextMenu") {
            QMenu *contextMenu = event.property("menu").value<QMenu *>();
            if (!contextMenu)
                return;

            QMetaObject::invokeMethod(this, [contextMenu](){
                contextMenu->addMenu(Copilot::instance()->getMenu());
            });
        } else if (eventData == "keyPressEvent") {
            int eventType = event.property("event").toInt();

            Copilot::instance()->processKeyPressEvent(static_cast<Qt::Key>(eventType));
        }
    }
}


