// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "currentprojectlocator.h"
#include "services/project/projectservice.h"
#include "services/editor/editorservice.h"

#include <QDebug>

using namespace dpfservice;
using Utils::FileName;

CurrentProjectLocator::CurrentProjectLocator(QObject *parent)
    : baseFileLocator(parent)
{
    setIncludedDefault(true);
    setDescription(tr("files in current project"));
    setDisplayName("p");
    setShortCut(QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_P));
}

void CurrentProjectLocator::prepareSearch(const QString &searchText)
{
    QList<QString> fileList {};
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService->getActiveProjectInfo) {
        auto project = projectService->getActiveProjectInfo();
        auto sourceFile = project.sourceFiles();
        fileList = sourceFile.values();
    }

    setFileList(fileList);
    baseFileLocator::prepareSearch(searchText);
}
