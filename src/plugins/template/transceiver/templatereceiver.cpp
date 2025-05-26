// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "templatereceiver.h"
#include "templatemanager.h"
#include "common/common.h"

TemplateReceiver::TemplateReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<TemplateReceiver> ()
{

}

dpf::EventHandler::Type TemplateReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList TemplateReceiver::topics()
{
    return { projectTemplate.topic };
}

void TemplateReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == projectTemplate.newWizard.name) {
        TemplateManager::instance()->newWizard();
    }
}
