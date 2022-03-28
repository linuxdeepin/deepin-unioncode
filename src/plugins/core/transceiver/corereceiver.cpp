/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
    return QStringList() << T_NAV;
}

void CoreReceiver::eventProcess(const dpf::Event &event)
{
    if (!CoreReceiver::topics().contains(event.topic())) {
        qCritical() << "Fatal error, unsubscribed message received";
        abort();
    }

    if (event.topic() == T_NAV)
        navEvent(event);
}

void CoreReceiver::navEvent(const dpf::Event &event)
{
    qInfo() << __FUNCTION__ << event;
    WindowKeeper::instace()->switchNavWidget(event.data().toString());
}
