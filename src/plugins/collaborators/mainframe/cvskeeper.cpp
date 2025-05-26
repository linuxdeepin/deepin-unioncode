// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
