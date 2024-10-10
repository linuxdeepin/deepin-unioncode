// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventreceiver.h"
#include "common/common.h"

extern const char *D_TSET;

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
    QString data = event.data().toString();
    QMetaObject::invokeMethod(this, [event](){
        if (event.topic() == T_MENU && event.data() == D_TSET) {
        QString displayText = event.property(P_ACTION_TEXT).toString();
        QMessageBox box;
        box.setText(displayText);
        box.exec();
    }});
}
