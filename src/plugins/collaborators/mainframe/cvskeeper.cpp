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
    if (gitReposWidget->isGitDir(repoPath))
        gitReposWidget->setRepositories({repoPath});
}

SvnClientWidget *CVSkeeper::svnMainWidget()
{
    return svnReposWidget;
}

GitQlientWidget *CVSkeeper::gitMainWidget()
{
    return gitReposWidget;
}
