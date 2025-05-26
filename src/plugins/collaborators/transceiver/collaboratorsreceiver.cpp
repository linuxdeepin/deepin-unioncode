// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collaboratorsreceiver.h"
#include "mainframe/cvskeeper.h"

#include "common/common.h"

CollaboratorsReceiver::CollaboratorsReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<CollaboratorsReceiver> ()
{

}

dpf::EventHandler::Type CollaboratorsReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList CollaboratorsReceiver::topics()
{
    return {T_COLLABORATORS};
}

void CollaboratorsReceiver::eventProcess(const dpf::Event &event)
{
    if (!topics().contains(event.topic()))
        abort();

    if (D_OPEN_REPOS == event.data()) {
        CVSkeeper::instance()->openRepos(event.property(P_WORKSPACEFOLDER).toString());
    }
}

