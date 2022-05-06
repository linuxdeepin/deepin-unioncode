#include "gitqlientwidget.h"

#include <QDir>

GitQlientWidget::GitQlientWidget(QWidget *parent)
    : GitQlient(parent)
{

}

bool GitQlientWidget::isGitDir(const QString &repoPath)
{
    QDir dir(repoPath + QDir::separator() + ".git");
    return dir.exists();
}
