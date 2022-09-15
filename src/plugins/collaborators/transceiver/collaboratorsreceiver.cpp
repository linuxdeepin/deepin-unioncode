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

