// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ninjaprojectgenerator.h"

#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

NinjaProjectGenerator::NinjaProjectGenerator()
{
    qRegisterMetaType<QList<QStandardItem*>>("QList<QStandardItem*>");
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

NinjaProjectGenerator::~NinjaProjectGenerator()
{
    qInfo() << __FUNCTION__;
}

QStringList NinjaProjectGenerator::supportLanguages()
{
    return {dpfservice::MWMFA_CXX};
}

QStringList NinjaProjectGenerator::supportFileNames()
{
    return {"build.ninja"};
}

QMenu *NinjaProjectGenerator::createItemMenu(const QStandardItem *item)
{
    Q_UNUSED(item)
    return nullptr;
}
