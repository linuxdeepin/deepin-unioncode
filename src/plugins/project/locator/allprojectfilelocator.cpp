// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "allprojectfilelocator.h"
#include "services/project/projectservice.h"
#include "services/editor/editorservice.h"

#include <QDebug>

using namespace dpfservice;
using Utils::FileName;

AllProjectFileLocator::AllProjectFileLocator(QObject *parent)
    : baseFileLocator(parent)
{
    setIncludedDefault(true);
    setDescription(tr("files in all project"));
    setDisplayName("a");
}

void AllProjectFileLocator::prepareSearch(const QString &searchText)
{
    QList<QString> fileList {};
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService->getAllProjectInfo) {
        auto allProject = projectService->getAllProjectInfo();
        foreach (auto project, allProject) {
            auto sourceFile = project.sourceFiles();
            fileList += sourceFile.values();
        }
    }
    setFileList(fileList);
    baseFileLocator::prepareSearch(searchText);
}
