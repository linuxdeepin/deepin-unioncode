// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "corereceiver.h"
#include "mainframe/windowkeeper.h"
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
    return {navigation.topic};
}

void CoreReceiver::eventProcess(const dpf::Event &event)
{
    if(event.data() == navigation.doSwitch.name) {
        QString actionTextKey = navigation.doSwitch.pKeys[0];
        QString actionText = event.property(actionTextKey).toString();
        QMetaObject::invokeMethod(this, [=](){
            WindowKeeper::instace()->switchWidgetNavigation(actionText);
        } , Qt::QueuedConnection);
    }
}
