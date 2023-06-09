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


