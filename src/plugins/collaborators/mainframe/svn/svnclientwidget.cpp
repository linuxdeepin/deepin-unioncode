#include "svnclientwidget.h"
#include "reposwidget.h"

#include "QPinnableTabWidget.h"
#include "GitQlientStyles.h"
#include "QLogger.h"

#include <QMenu>
#include <QEvent>
#include <QPushButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QLabel>

#include "FileEditor.h"
#include "FileDiffView.h"

SvnClientWidget::SvnClientWidget(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow (parent, flags)
    , mRepos(new QPinnableTabWidget())
{
    mRepos = new QPinnableTabWidget();
    const auto homeMenu = new QPushButton();
    const auto menu = new QMenu(homeMenu);
    menu->installEventFilter(this);

    homeMenu->setIcon(QIcon(":/icons/burger_menu"));
    homeMenu->setIconSize(QSize(17, 17));
    homeMenu->setToolTip("Options");
    homeMenu->setMenu(menu);
    homeMenu->setObjectName("MainMenuBtn");
    mRepos->setObjectName("GitQlientTab");
    mRepos->setStyleSheet(GitQlientStyles::getStyles());
    mRepos->setCornerWidget(homeMenu, Qt::TopLeftCorner);
    setCentralWidget(mRepos);
//    addRepoTab("/home/funning/Documents/svn");
//    auto diffWidget = new FileDiffView();
//    auto text = "123\n1234\n12345\n123456\n1234567\n12345678\n";
//    ChunkDiffInfo::ChunkInfo info1{};
//    info1.startLine = 0;
//    info1.endLine = 1;
//    info1.addition = false;
//    ChunkDiffInfo::ChunkInfo info2{};
//    info2.startLine = 3;
//    info2.endLine  = 4;
//    info2.addition = true;
//    diffWidget->loadDiff(text, {info1});
//    diffWidget->loadDiff(text, {info2});
//    diffWidget->show();
}

void SvnClientWidget::addRepoTab(const QString &repoPath)
{
    addNewRepoTab(repoPath, false);
}

void SvnClientWidget::addNewRepoTab(const QString &repoPathArg, bool pinned)
{
    const auto repoPath = QFileInfo(repoPathArg).canonicalFilePath();

    if (!mCurrentRepos.contains(repoPath)) {
        const auto repoName = repoPath.contains("/") ? repoPath.split("/").last() : "";
        auto reposWidget = new ReposWidget;
        reposWidget->setReposPath(repoPathArg);
        const int index = mRepos->addPinnedTab(reposWidget, repoName);
        mRepos->setTabIcon(index, QIcon(QString(":/icons/local")));
    }
}


bool SvnClientWidget::eventFilter(QObject *obj, QEvent *event)
{

    const auto menu = qobject_cast<QMenu *>(obj);
    if (menu && event->type() == QEvent::Show) {
        auto localPos = menu->parentWidget()->pos();
        auto pos = mapToGlobal(localPos);
        menu->show();
        pos.setY(pos.y() + menu->parentWidget()->height());
        menu->move(pos);
        return true;
    }

    return false;
}
