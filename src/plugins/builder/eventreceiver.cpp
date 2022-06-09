/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#include "eventreceiver.h"
#include "common/common.h"
#include "buildersignals.h"
#include "builderglobals.h"
#include "buildmanager.h"

static EventReceiver *ins = nullptr;

EventReceiver::EventReceiver(QObject *parent)
    : dpf::EventHandler(parent)
    , dpf::AutoEventHandlerRegister<EventReceiver>()
{
    ins = this;
}

EventReceiver *EventReceiver::instance()
{
    return ins;
}

dpf::EventHandler::Type EventReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList EventReceiver::topics()
{
    return { T_DEBUGGER };
}

const QString &EventReceiver::projectFilePath() const
{
    return proFilePath;
}

const QString &EventReceiver::projectDirectory() const
{
    return proDirPath;
}

const QString &EventReceiver::rootProjectDirectory() const
{
    return proRootPath;
}

const QString &EventReceiver::buildOutputDirectory() const
{
    return buildOutputPath;
}

ToolChainType EventReceiver::toolChainType() const
{
    return tlChainType;
}

void EventReceiver::eventProcess(const dpf::Event &event)
{
    if (!topics().contains(event.topic())) {
        qDebug() << "Fatal";
        return;
    }

    qInfo() << event;

    QString topic = event.topic();
    QString data = event.data().toString();

     if (topic == T_DEBUGGER) {
        if (data == D_DEBUG_EXECUTION_START) {
            BuildManager::instance()->buildProject();
        }
    }
}

