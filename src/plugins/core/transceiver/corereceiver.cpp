// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "corereceiver.h"
#include "uicontroller/controller.h"
#include "common/common.h"

#include <QGridLayout>
#include <QActionGroup>

CoreReceiver::CoreReceiver(QObject *parent)
    : dpf::EventHandler (parent)
{

}

dpf::EventHandler::Type CoreReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList CoreReceiver::topics()
{
    return {uiController.topic};
}

void CoreReceiver::eventProcess(const dpf::Event &event)
{
    if(event.data() == uiController.doSwitch.name) {
        QString actionTextKey = uiController.doSwitch.pKeys[0];
        QString actionText = event.property(actionTextKey).toString();
        QMetaObject::invokeMethod(this, [=](){
            Controller::instance()->switchWidgetNavigation(actionText);
        } , Qt::QueuedConnection);
    } else if(event.data() == uiController.switchContext.name) {
        QString titleName = event.property(uiController.switchContext.pKeys[0]).toString();
        QMetaObject::invokeMethod(this, [=](){
            Controller::instance()->switchContextWidget(titleName);
        } , Qt::QueuedConnection);
    } else if(event.data() == uiController.switchWorkspace.name) {
        QString titleName = event.property(uiController.switchWorkspace.pKeys[0]).toString();
        Controller::instance()->switchWorkspace(titleName);
    }
}
