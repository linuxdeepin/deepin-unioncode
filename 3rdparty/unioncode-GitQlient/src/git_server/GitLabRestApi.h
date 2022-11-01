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

#include <IRestApi.h>

namespace GitServer
{

class GitLabRestApi final : public IRestApi
{
   Q_OBJECT

public:
   explicit GitLabRestApi(const QString &userName, const QString &repoName, const QString &settingsKey,
                          const ServerAuthentication &auth, QObject *parent = nullptr);

   QString getUserName() const override { return mUserId; }

   void testConnection() override;
   void createIssue(const Issue &issue) override;
   void updateIssue(int issueNumber, const Issue &issue) override;
   void updatePullRequest(int, const PullRequest &) override { }
   void createPullRequest(const PullRequest &pr) override;
   void requestLabels() override;
   void requestMilestones() override;
   void requestIssues(int) override;
   void requestPullRequests(int) override;
   void mergePullRequest(int, const QByteArray &) override { }
   void requestComments(int) override { }
   void requestReviews(int) override { }
   void requestCommitsFromPR(int) override { }

   QString getUserId() const { return mUserId; }

private:
   QString mUserName;
   QString mRepoName;
   QString mSettingsKey;
   QString mUserId;
   QString mRepoId;
   int mPreRequisites = -1;
   bool mTestRequested = false;

   QNetworkRequest createRequest(const QString &page) const override;

   void getUserInfo() const;
   void onUserInfoReceived();
   void getProjects();
   void onProjectsReceived();
   void onLabelsReceived();
   void onMilestonesReceived();
   void onIssueCreated();
   void onIssueReceived();
   void onMergeRequestCreated();

   Issue issueFromJson(const QJsonObject &json) const;
   PullRequest prFromJson(const QJsonObject &json) const;
};

}
