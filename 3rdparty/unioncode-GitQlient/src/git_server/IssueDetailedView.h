#pragma once

/****************************************************************************************
 ** GitQlient is an application to manage and operate one or several Git repositories. With
 ** GitQlient you will be able to add commits, branches and manage all the options Git provides.
 ** Copyright (C) 2021  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This program is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 2 of the License, or (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/

#include <QFrame>
#include <QMap>

#include <Issue.h>

class QLabel;
class GitServerCache;
class QButtonGroup;
class PrCommentsList;
class PrCommitsList;
class QStackedLayout;
class GitBase;
class PrChangesList;
class QToolButton;
class QPushButton;

namespace GitServer
{
struct Issue;
struct PullRequest;
}

class IssueDetailedView : public QFrame
{
   Q_OBJECT
signals:
   void openDiff(const QString &sha);

public:
   enum class Config
   {
      Issues,
      PullRequests
   };
   explicit IssueDetailedView(const QSharedPointer<GitBase> &git, const QSharedPointer<GitServerCache> &gitServerCache,
                              QWidget *parent = nullptr);
   ~IssueDetailedView();

   void loadData(Config config, int issueNum, bool force = false);

protected:
   bool eventFilter(QObject *obj, QEvent *event);

private:
   enum class Buttons
   {
      Comments,
      Changes,
      Commits
   };

   GitServer::Issue mIssue;
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitServerCache> mGitServerCache;
   Config mConfig;
   int mIssueNumber = -1;
   QButtonGroup *mBtnGroup = nullptr;
   QLabel *mTitleLabel = nullptr;
   QStackedLayout *mStackedLayout = nullptr;
   PrCommentsList *mPrCommentsList = nullptr;
   PrChangesList *mPrChangesList = nullptr;
   PrCommitsList *mPrCommitsList = nullptr;
   QToolButton *mReviewBtn = nullptr;
   QPushButton *mAddComment = nullptr;
   QPushButton *mCloseIssue = nullptr;

   void onViewChange(int viewId);
   void closeIssue();
   void openAddReviewDlg(QAction *sender);
   void addReview(const QString &body, const QString &mode);
   void addCodeReview(int line, const QString &path, const QString &body);
};
