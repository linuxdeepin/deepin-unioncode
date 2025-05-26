// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gitqlientwidget.h"
#include "services/project/projectservice.h"

#include <QDir>
#include <QTimer>

using namespace dpfservice;
GitQlientWidget::GitQlientWidget(QWidget *parent)
    : GitQlient(parent)
{

}

bool GitQlientWidget::isGitDir(const QString &repoPath)
{
    if (repoPath.isEmpty())
        return false;

    QDir dir(repoPath + QDir::separator() + ".git");
    return dir.exists();
}

QString GitQlientWidget::getRepositoryPath() const
{
    auto activeProjInfo = dpfGetService(ProjectService)->getActiveProjectInfo();
    return activeProjInfo.workspaceFolder();
}

void GitQlientWidget::showEvent(QShowEvent *event)
{
    GitQlient::showEvent(event);

    int delayTimeToAvoidBlock = 1;
    QTimer::singleShot(delayTimeToAvoidBlock, this, [this] {
        QString repositoryPath = getRepositoryPath();
        if (isGitDir(repositoryPath))
            setRepositories({repositoryPath});
    });
}


