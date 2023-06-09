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
#include "cvskeeper.h"

#include "git/gitqlientwidget.h"
#include "svn/svnclientwidget.h"

CVSkeeper::CVSkeeper(QObject *parent)
    : QObject(parent)
    , svnReposWidget(new SvnClientWidget)
    , gitReposWidget(new GitQlientWidget)
{

}

CVSkeeper *CVSkeeper::instance(){
    static CVSkeeper ins;
    return &ins;
}

void CVSkeeper::openRepos(const QString &repoPath)
{
    if (svnReposWidget->isSvnDir(repoPath))
        svnReposWidget->addNewRepoTab(repoPath);
}

SvnClientWidget *CVSkeeper::svnMainWidget()
{
    return svnReposWidget;
}

GitQlientWidget *CVSkeeper::gitMainWidget()
{
    return gitReposWidget;
}
