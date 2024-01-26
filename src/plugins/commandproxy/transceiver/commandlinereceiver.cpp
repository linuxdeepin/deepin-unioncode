// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commandlinereceiver.h"
#include "commandexecuter.h"

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
