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

#include <Milestone.h>
#include <Label.h>
#include <PullRequest.h>

#include <QObject>
#include <QMap>
#include <QNetworkRequest>

class QNetworkAccessManager;
class QNetworkReply;

namespace GitServer
{

struct Issue;

struct ServerAuthentication
{
   QString userName;
   QString userPass;
   QString endpointUrl;
};

class IRestApi : public QObject
{
   Q_OBJECT

signals:
   /**
    * @brief connectionTested Signal triggered when the connection to the remote Git server succeeds.
    */
   void connectionTested();
   /**
    * @brief labelsReceived Signal triggered after the labels are received and processed.
    * @param labels The processed labels.
    */
   void labelsReceived(const QVector<GitServer::Label> &labels);
   /**
    * @brief milestonesReceived Signal triggered after the milestones are received and processed.
    * @param milestones The processed milestones.
    */
   void milestonesReceived(const QVector<GitServer::Milestone> &milestones);

   /**
    * @brief issuesReceived Signal triggered when the issues has been received.
    * @param issues The list of issues.
    */
   void issuesReceived(const QVector<GitServer::Issue> &issues);

   /**
    * @brief pullRequestsReceived Signal triggered when the pull requests has been received.
    * @param prs The list of prs.
    */
   void pullRequestsReceived(const QVector<GitServer::PullRequest> &prs);

   /**
    * @brief pullRequestMerged Signal triggered when the pull request has been merged.
    */
   void pullRequestMerged();

   /**
    * @brief errorOccurred Signal triggered when an error happened.
    * @param errorStr The error in string format.
    */
   void errorOccurred(const QString &errorStr);

   /**
    * @brief paginationPresent Signal triggered when the issues or pull requests are so many that they are sent
    * paginated.
    * @param current The current page.
    * @param next The next page.
    * @param total The total of pages.
    */
   void paginationPresent(int current, int next, int total);

   /**
    * @brief issueCreated Signal triggered when an issue has been created.
    * @param url The url of the issue.
    */
   void issueUpdated(const GitServer::Issue &issue);

   /**
    * @brief commentsReceived Signal triggered when comments for an issue has been received.
    * @param issueNumber The number of the issue.
    * @param comments The list of comments.
    */
   void commentsReceived(int issueNumber, const QVector<GitServer::Comment> &comments);

   /**
    * @brief onCodeReviewsReceived Signal triggered when code reviews for a PR has been received.
    * @param prNumber The number of the PR.
    * @param codeReviews The code reviews.
    */
   void codeReviewsReceived(int prNumber, const QVector<GitServer::CodeReview> &codeReviews);

   /**
    * @brief onCommentReviewsReceived Signal triggered when the review comments for a PR has been received.
    * @param prNumber The number of the PR.
    * @param commentReviews The comment reviews.
    */
   void commentReviewsReceived(int prNumber, const QMap<int, GitServer::Review> &commentReviews);

   /**
    * @brief commitsReceived Signal triggered when the commits of a PR has been received.
    * @param prNumber The number of the PR.
    * @param commits The commits.
    */
   void commitsReceived(int prNumber, const QVector<GitServer::Commit> &commits, int currentPage, int lastPage);

   /**
    * @brief pullRequestUpdated Signal triggered when a pull request has been updated.
    * @param pr The updated pull request.
    */
   void pullRequestUpdated(const GitServer::PullRequest &pr);

public:
   explicit IRestApi(const ServerAuthentication &auth, QObject *parent = nullptr);
   virtual ~IRestApi();

   virtual QString getUserName() const { return mAuth.userName; }

   static QJsonDocument validateData(QNetworkReply *reply, QString &errorString);

   /**
    * @brief testConnection Tests the connection against the server.
    */
   virtual void testConnection() = 0;
   /**
    * @brief createIssue Creates a new issue in the remote Git server.
    * @param issue The informatio of the issue.
    */
   virtual void createIssue(const Issue &issue) = 0;
   /**
    * @brief updateIssue Updates an existing issue or pull request, if it doesn't exist it reports an error.
    * @param issueNumber The issue number to update.
    * @param issue The updated information of the issue.
    */
   virtual void updateIssue(int issueNumber, const Issue &issue) = 0;

   /**
    * @brief updatePullRequest Updates a pull request.
    * @param number The number of the PR
    * @param pr The pr to extract the data
    */
   virtual void updatePullRequest(int number, const PullRequest &pr) = 0;
   /**
    * @brief createPullRequest Creates a pull request in the remote Git server.
    * @param pullRequest The information of the pull request.
    */
   virtual void createPullRequest(const PullRequest &pullRequest) = 0;
   /**
    * @brief requestLabels Requests the labels to the remote Git server.
    */
   virtual void requestLabels() = 0;
   /**
    * @brief requestMilestones Requests the milestones to the remote Git server.
    */
   virtual void requestMilestones() = 0;
   /**
    * @brief requestIssues Requests the issues to the remote Git server.
    */
   virtual void requestIssues(int page = -1) = 0;

   /**
    * @brief requestPullRequests Requests the pull request to the remote Git server.
    */
   virtual void requestPullRequests(int page = -1) = 0;
   /**
    * @brief mergePullRequest Merges a pull request into the destination branch.
    * @param number The number of the pull request.
    * @param data Byte array in JSON format with the necessary data to merge the pull request.
    */
   virtual void mergePullRequest(int number, const QByteArray &data) = 0;

   /**
    * @brief requestComments Requests all the comments of an issue. This doesn't get the reviews and comments on reviews
    * for a pull request.
    * @param issueNumber The issue number to query.
    */
   virtual void requestComments(int issueNumber) = 0;

   /**
    * @brief requestReviews Requests all the reviews in a Pull Requests.
    * @param pr The Pull Request to query.
    */
   virtual void requestReviews(int prNumber) = 0;

   /**
    * @brief requestCommitsFromPR Requests all the commits from a PR.
    * @param prNumber The Pr number.
    */
   virtual void requestCommitsFromPR(int prNumber) = 0;

   /**
    * @brief addIssueComment Adds a comment to an issue or PR.
    */
   virtual void addIssueComment(const Issue &, const QString &) { }

   virtual void addPrReview(int, const QString &, const QString &) { }

   virtual void addPrCodeReview(int, const QString &, const QString &, int, const QString &) { }

   virtual void replyCodeReview(int, int, const QString &) { }

protected:
   QNetworkAccessManager *mManager = nullptr;
   ServerAuthentication mAuth;

   /**
    * @brief createRequest Creates a request to be consumed by the Git remote server.
    * @param page The destination page of the request.
    * @return Returns a QNetworkRequest object with the configuration needed by the server.
    */
   virtual QNetworkRequest createRequest(const QString &page) const = 0;
};

}
