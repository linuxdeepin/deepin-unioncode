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
#include <ConfigData.h>

class GitCache;
class GitBase;
class GitServerCache;
class IssueDetailedView;
class QPushButton;
class QStackedLayout;
//class CreateIssueDlg;
class CreatePullRequestDlg;

namespace GitServer
{
class IRestApi;
struct Issue;
}

class GitServerWidget : public QFrame
{
   Q_OBJECT

signals:
   void openDiff(const QString &sha);

public:
   explicit GitServerWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                            const QSharedPointer<GitServerCache> &gitServerCache, QWidget *parent = nullptr);

   ~GitServerWidget();

   /**
    * @brief configure Configures the widget by showing the config dialog or the full content if it was already
    * configured.
    * @return Returns true if configured, otherwise false.
    */
   bool configure(const GitServer::ConfigData &config = GitServer::ConfigData());

   /**
    * @brief isConfigured Returns the current state of the widget
    * @return True if configured, otherwise false.
    */
   bool isConfigured() const { return mConfigured; }

   /**
    * @brief openPullRequest The method opens the PR view directly.
    * @param prNumber The PR number.
    */
   void openPullRequest(int prNumber);

private:
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitServerCache> mGitServerCache;
   QStackedLayout *mStackedLayout = nullptr;
   IssueDetailedView *mDetailedView = nullptr;
   QFrame *mGeneralView = nullptr;
//   CreateIssueDlg *mCreateIssueView = nullptr;
   CreatePullRequestDlg *mCreatePrView = nullptr;
   QPushButton *mOldIssue = nullptr;
   QPushButton *mOldPr = nullptr;
   QPushButton *mRefresh = nullptr;
   bool mConfigured = false;

   /**
    * @brief createWidget Creates all the contents of the GitServerWidget.
    */
   void createWidget();
};
