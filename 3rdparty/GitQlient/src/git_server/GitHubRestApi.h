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

#include <QUrl>
#include <QNetworkRequest>

class QJsonDocument;
class QNetworkReply;

namespace GitServer
{

struct Issue;

class GitHubRestApi final : public IRestApi
{
   Q_OBJECT

public:
   explicit GitHubRestApi(QString repoOwner, QString repoName, const ServerAuthentication &auth,
                          QObject *parent = nullptr);

   void testConnection() override;
   void createIssue(const Issue &issue) override;
   void updateIssue(int issueNumber, const Issue &issue) override;
   void updatePullRequest(int number, const PullRequest &pr) override;
   void createPullRequest(const PullRequest &pullRequest) override;
   void requestLabels() override;
   void requestMilestones() override;
   void requestIssues(int page = -1) override;
   void requestPullRequests(int page = -1) override;
   void mergePullRequest(int number, const QByteArray &data) override;
   void requestComments(int issueNumber) override;
   void requestReviews(int prNumber) override;
   void requestCommitsFromPR(int prNumber) override;
   void addIssueComment(const Issue &issue, const QString &text) override;
   void addPrReview(int prNumber, const QString &body, const QString &event) override;
   void addPrCodeReview(int prNumber, const QString &body, const QString &path, int pos, const QString &sha) override;
   void replyCodeReview(int prNumber, int commentId, const QString &msgBody) override;

private:
   QString mRepoEndpoint;
   QByteArray mAuthString;

   QNetworkRequest createRequest(const QString &page) const override;
   void onLabelsReceived();
   void onMilestonesReceived();
   void onIssueCreated();
   void onPullRequestCreated();
   void onPullRequestMerged();
   void onPullRequestReceived();
   void onPullRequestStatusReceived(PullRequest pr);
   void onIssuesReceived();
   void onCommentsReceived(int issueNumber);
   void onPullRequestDetailsReceived(PullRequest pr);
   void onReviewsReceived(int prNumber);

   void requestReviewComments(int prNumber);
   void onReviewCommentsReceived(int prNumber);
   void onCommitsReceived(int prNumber);

   Issue issueFromJson(const QJsonObject &json) const;
   PullRequest prFromJson(const QJsonObject &json) const;
};

}
