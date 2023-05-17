/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<huangyub@uniontech.com>
 *
 * Maintainer: hongjinchuan<huangyub@uniontech.com>
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

#include "commandlinereceiver.h"
#include "commandexecuter.h"

#include "services/project/projectinfo.h"
#include "services/project/projectservice.h"
#include "services/window/windowelement.h"
#include "services/project/projectgenerator.h"
#include "services/builder/builderglobals.h"
#include "common/common.h"

CommandLineReceiver::CommandLineReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<CommandLineReceiver> ()
{

}

dpf::EventHandler::Type CommandLineReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList CommandLineReceiver::topics()
{
    return {commandLine.topic, commandLine.topic};
}

void CommandLineReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == commandLine.build.name) {
        CommandExecuter::instance().buildProject();
    }
}
