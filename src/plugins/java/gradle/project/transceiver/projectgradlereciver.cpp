// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectgradlereciver.h"
#include "common/common.h"

#include "services/project/projectservice.h"
#include "services/window/windowelement.h"
#include "services/project/projectgenerator.h"
#include "services/builder/builderglobals.h"

ProjectGradleReceiver::ProjectGradleReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<ProjectGradleReceiver> ()
{

}

dpf::EventHandler::Type ProjectGradleReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ProjectGradleReceiver::topics()
{
    return { T_BUILDER, project.topic};
}

void ProjectGradleReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == project.projectNodeExpanded.name) {
        auto index = event.property("modelIndex").value<QModelIndex>();
        auto filePath = index.data(Qt::ToolTipRole).toString();
        emit ProjectGradleProxy::instance()->nodeExpanded(filePath);
    }

    if (event.data() == project.projectNodeCollapsed.name) {
        auto index = event.property("modelIndex").value<QModelIndex>();
        auto filePath = index.data(Qt::ToolTipRole).toString();
        emit ProjectGradleProxy::instance()->nodeCollapsed(filePath);
    }
}

ProjectGradleProxy *ProjectGradleProxy::instance()
{
    static ProjectGradleProxy ins;
    return &ins;
}
