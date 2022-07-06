/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "gradlereceiver.h"
#include "mainframe/gradlemanager.h"
#include "common/common.h"

static GradleReceiver *ins = nullptr;

GradleReceiver::GradleReceiver(QObject *parent)
    : dpf::EventHandler(parent)
    , dpf::AutoEventHandlerRegister<GradleReceiver>()
{
    ins = this;
}

GradleReceiver *GradleReceiver::instance()
{
    return ins;
}

dpf::EventHandler::Type GradleReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList GradleReceiver::topics()
{
    return { T_DEBUGGER, T_BUILDER };
}

void GradleReceiver::eventProcess(const dpf::Event &event)
{
    if (!topics().contains(event.topic())) {
        qDebug() << "Fatal";
        return;
    }

    QString topic = event.topic();
    QString data = event.data().toString();

    if (topic == T_DEBUGGER) {
        if (data == D_DEBUG_EXECUTION_START) {
            //BuildManager::instance()->buildProject();
        }
    }
    else if(topic == T_BUILDER) {
        if (data == D_BUILD_COMMAND) {
            QString program = event.property(P_BUILDPROGRAM).toString();
            QStringList arguments = event.property(P_BUILDARGUMENTS).toStringList();
            QString workingdir = event.property(P_BUILDWORKINGDIR).toString();

            GradleManager::instance()->executeBuildCommand(program, arguments, workingdir);
        } else if (data == D_MENU_BUILD) {
            GradleManager::instance()->buildProject();
        } else if (data == D_MENU_REBUILD) {
            GradleManager::instance()->rebuildProject();
        } else if (data == D_MENU_CLEAN) {
            GradleManager::instance()->cleanProject();
        }
    }
}

