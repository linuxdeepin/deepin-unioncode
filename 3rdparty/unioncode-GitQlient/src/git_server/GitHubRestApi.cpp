#include "GitHubRestApi.h"
#include <Issue.h>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QUrlQuery>

#include <QLogger.h>

using namespace QLogger;
using namespace GitServer;

GitHubRestApi::GitHubRestApi(QString repoOwner, QString repoName, const ServerAuthentication &auth, QObject *parent)
   : IRestApi(auth, parent)
{
   if (!repoOwner.endsWith("/"))
      repoOwner.append("/");

   if (!repoOwner.startsWith("/"))
      repoOwner.prepend("/");

   if (repoName.endsWith("/"))
      repoName = repoName.left(repoName.size() - 1);

   mRepoEndpoint = QString("/repos") + repoOwner + repoName;

   mAuthString = "Basic "
       + QByteArray(QString(QStringLiteral("%1:%2")).arg(mAuth.userName, mAuth.userPass).toLocal8Bit()).toBase64();
}

void GitHubRestApi::testConnection()
{
   auto request = createRequest("/user/repos");

   const auto reply = mManager->get(request);

   connect(reply, &QNetworkReply::finished, this, [this]() {
      const auto reply = qobject_cast<QNetworkReply *>(sender());
      QString errorStr;
      const auto tmpDoc = validateData(reply, errorStr);

      if (!tmpDoc.isEmpty())
         emit connectionTested();
      else
         emit errorOccurred(errorStr);
   });
}

void GitHubRestApi::createIssue(const Issue &issue)
{
   QJsonDocument doc(issue.toJson());
   const auto data = doc.toJson(QJsonDocument::Compact);

   auto request = createRequest(mRepoEndpoint + "/issues");
   request.setRawHeader("Content-Length", QByteArray::number(data.size()));
   const auto reply = mManager->post(request, data);

   connect(reply, &QNetworkReply::finished, this, &GitHubRestApi::onIssueCreated);
}

void GitHubRestApi::updateIssue(int issueNumber, const Issue &issue)
{
   QJsonDocument doc(issue.toJson());
   const auto data = doc.toJson(QJsonDocument::Compact);

   auto request = createRequest(QString(mRepoEndpoint + "/issues/%1").arg(issueNumber));
   request.setRawHeader("Content-Length", QByteArray::number(data.size()));
   request.setRawHeader("Accept", "application/vnd.github.v3+json");
   const auto reply = mManager->post(request, data);

   connect(reply, &QNetworkReply::finished, this, [this]() {
      const auto reply = qobject_cast<QNetworkReply *>(sender());
      QString errorStr;
      const auto tmpDoc = validateData(reply, errorStr);

      if (const auto issueObj = tmpDoc.object(); !issueObj.contains("pull_request"))
      {
         const auto issue = issueFromJson(issueObj);
         emit issueUpdated(issue);
      }
      else
         emit errorOccurred(errorStr);
   });
}

void GitHubRestApi::updatePullRequest(int number, const PullRequest &pr)
{
   QJsonDocument doc(Issue(pr).toJson());
   const auto data = doc.toJson(QJsonDocument::Compact);

   auto request = createRequest(QString(mRepoEndpoint + "/issues/%1").arg(number));
   request.setRawHeader("Content-Length", QByteArray::number(data.size()));
   const auto reply = mManager->post(request, data);

   connect(reply, &QNetworkReply::finished, this, [this]() {
      const auto reply = qobject_cast<QNetworkReply *>(sender());
      QString errorStr;
      const auto tmpDoc = validateData(reply, errorStr);

      if (const auto issueObj = tmpDoc.object(); issueObj.contains("pull_request"))
      {
         const auto pr = prFromJson(issueObj);
         emit pullRequestUpdated(pr);
      }
      else
         emit errorOccurred(errorStr);
   });
}

void GitHubRestApi::createPullRequest(const PullRequest &pullRequest)
{
   QJsonDocument doc(pullRequest.toJson());
   const auto data = doc.toJson(QJsonDocument::Compact);

   auto request = createRequest(mRepoEndpoint + "/pulls");
   request.setRawHeader("Content-Length", QByteArray::number(data.size()));

   const auto reply = mManager->post(request, data);
   connect(reply, &QNetworkReply::finished, this, [this]() {
      const auto reply = qobject_cast<QNetworkReply *>(sender());
      QString errorStr;
      const auto tmpDoc = validateData(reply, errorStr);

      if (!tmpDoc.isEmpty())
      {
         const auto pr = prFromJson(tmpDoc.object());
         emit pullRequestUpdated(pr);

         updatePullRequest(pr.number, pr);
      }
   });
}

void GitHubRestApi::requestLabels()
{
   const auto reply = mManager->get(createRequest(mRepoEndpoint + "/labels"));

   connect(reply, &QNetworkReply::finished, this, &GitHubRestApi::onLabelsReceived);
}

void GitHubRestApi::requestMilestones()
{
   const auto reply = mManager->get(createRequest(mRepoEndpoint + "/milestones"));

   connect(reply, &QNetworkReply::finished, this, &GitHubRestApi::onMilestonesReceived);
}

void GitHubRestApi::requestIssues(int page)
{
   auto request = createRequest(mRepoEndpoint + "/issues");
   auto url = request.url();
   QUrlQuery query;

   if (page != -1)
   {
      query.addQueryItem("page", QString::number(page));
      url.setQuery(query);
   }

   query.addQueryItem("per_page", QString::number(100));
   url.setQuery(query);

   request.setUrl(url);

   const auto reply = mManager->get(request);

   connect(reply, &QNetworkReply::finished, this, &GitHubRestApi::onIssuesReceived);
}

void GitHubRestApi::requestPullRequests(int page)
{
   auto request = createRequest(mRepoEndpoint + "/pulls");
   auto url = request.url();
   QUrlQuery query;

   if (page != -1)
   {
      query.addQueryItem("page", QString::number(page));
      url.setQuery(query);
   }

   query.addQueryItem("per_page", QString::number(100));
   url.setQuery(query);

   request.setUrl(url);

   const auto reply = mManager->get(request);

   connect(reply, &QNetworkReply::finished, this, &GitHubRestApi::onPullRequestReceived);
}

void GitHubRestApi::mergePullRequest(int number, const QByteArray &data)
{
   const auto reply = mManager->put(createRequest(mRepoEndpoint + QString("/pulls/%1/merge").arg(number)), data);

   connect(reply, &QNetworkReply::finished, this, &GitHubRestApi::onPullRequestMerged);
}

void GitHubRestApi::requestComments(int issueNumber)
{
   const auto reply = mManager->get(createRequest(mRepoEndpoint + QString("/issues/%1/comments").arg(issueNumber)));

   connect(reply, &QNetworkReply::finished, this, [this, issueNumber]() { onCommentsReceived(issueNumber); });
}

void GitHubRestApi::requestReviews(int prNumber)
{
   const auto reply = mManager->get(createRequest(mRepoEndpoint + QString("/pulls/%1/reviews").arg(prNumber)));

   connect(reply, &QNetworkReply::finished, this, [this, prNumber]() { onReviewsReceived(prNumber); });
}

void GitHubRestApi::requestCommitsFromPR(int prNumber)
{
   auto request = createRequest(mRepoEndpoint + QString("/pulls/%1/commits").arg(prNumber));
   const auto reply = mManager->get(request);

   connect(reply, &QNetworkReply::finished, this, [this, prNumber]() { onCommitsReceived(prNumber); });
}

void GitHubRestApi::addIssueComment(const Issue &issue, const QString &text)
{
   QJsonObject object;
   object.insert("body", text);

   QJsonDocument doc(object);
   const auto data = doc.toJson(QJsonDocument::Compact);

   auto request = createRequest(QString(mRepoEndpoint + "/issues/%1/comments").arg(issue.number));
   request.setRawHeader("Content-Length", QByteArray::number(data.size()));
   request.setRawHeader("Accept", "application/vnd.github.v3+json");
   const auto reply = mManager->post(request, data);

   connect(reply, &QNetworkReply::finished, this, [this, issue]() {
      const auto reply = qobject_cast<QNetworkReply *>(sender());
      const auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
      QString errorStr;
      const auto tmpDoc = validateData(reply, errorStr);

      if (statusCode.isValid() && statusCode.toInt() == 201 && !tmpDoc.isEmpty())
      {
         const auto commentData = tmpDoc.object();
         auto newIssue = issue;
         newIssue.commentsCount += 1;

         Comment c;
         c.id = commentData["id"].toInt();
         c.body = commentData["body"].toString();
         c.creation = commentData["created_at"].toVariant().toDateTime();
         c.association = commentData["author_association"].toString();

         GitServer::User sAssignee;
         sAssignee.id = commentData["user"].toObject()["id"].toInt();
         sAssignee.url = commentData["user"].toObject()["html_url"].toString();
         sAssignee.name = commentData["user"].toObject()["login"].toString();
         sAssignee.avatar = commentData["user"].toObject()["avatar_url"].toString();
         sAssignee.type = commentData["user"].toObject()["type"].toString();

         c.creator = std::move(sAssignee);
         newIssue.comments.append(std::move(c));

         emit issueUpdated(newIssue);
      }
   });
}

void GitHubRestApi::addPrReview(int prNumber, const QString &body, const QString &event)
{
   QJsonObject object;
   object.insert("body", body);
   object.insert("pull_number", prNumber);
   object.insert("event", event);

   QJsonDocument doc(object);
   const auto data = doc.toJson(QJsonDocument::Compact);

   auto request = createRequest(QString(mRepoEndpoint + "/pulls/%1/reviews").arg(prNumber));
   request.setRawHeader("Content-Length", QByteArray::number(data.size()));
   request.setRawHeader("Accept", "application/vnd.github.v3+json");
   const auto reply = mManager->post(request, data);

   connect(reply, &QNetworkReply::finished, this, [this, prNumber]() {
      const auto reply = qobject_cast<QNetworkReply *>(sender());
      const auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
      QString errorStr;
      const auto tmpDoc = validateData(reply, errorStr);

      if (statusCode.isValid() && statusCode.toInt() == 201 && !tmpDoc.isEmpty())
      {
         const auto commentData = tmpDoc.object();
         Review r;
         r.id = commentData["id"].toInt();
         r.body = commentData["body"].toString();
         r.creation = commentData["submitted_at"].toVariant().toDateTime();
         r.state = commentData["state"].toString();
         r.association = commentData["author_association"].toString();

         GitServer::User sAssignee;
         sAssignee.id = commentData["user"].toObject()["id"].toInt();
         sAssignee.url = commentData["user"].toObject()["html_url"].toString();
         sAssignee.name = commentData["user"].toObject()["login"].toString();
         sAssignee.avatar = commentData["user"].toObject()["avatar_url"].toString();
         sAssignee.type = commentData["user"].toObject()["type"].toString();

         r.creator = std::move(sAssignee);

         emit commentReviewsReceived(prNumber, { { r.id, r } });
      }
   });
}

void GitHubRestApi::addPrCodeReview(int prNumber, const QString &body, const QString &path, int pos, const QString &sha)
{
   QJsonObject object;
   object.insert("body", body);
   object.insert("path", path);
   object.insert("line", pos);
   object.insert("commit_id", sha);

   QJsonDocument doc(object);
   const auto data = doc.toJson(QJsonDocument::Compact);

   auto request = createRequest(QString(mRepoEndpoint + "/pulls/%1/comments").arg(prNumber));
   request.setRawHeader("Content-Length", QByteArray::number(data.size()));
   request.setRawHeader("Accept", "application/vnd.github.v3+json");
   const auto reply = mManager->post(request, data);

   connect(reply, &QNetworkReply::finished, this, [this, prNumber]() {
      const auto reply = qobject_cast<QNetworkReply *>(sender());
      const auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
      QString errorStr;
      const auto tmpDoc = validateData(reply, errorStr);

      if (statusCode.isValid() && statusCode.toInt() == 201 && !tmpDoc.isEmpty())
      {
         const auto commentData = tmpDoc.object();
         CodeReview c;
         c.outdated = false;
         c.id = commentData["id"].toInt();
         c.body = commentData["body"].toString();
         c.creation = commentData["created_at"].toVariant().toDateTime();
         c.association = commentData["author_association"].toString();
         c.diff.diff = commentData["diff_hunk"].toString();
         c.diff.file = commentData["path"].toString();

         if (commentData.contains("line"))
            c.diff.line = commentData["line"].toInt();
         else
         {
            if (commentData["position"].toInt() != 0)
               c.diff.line = commentData["position"].toInt();
            else
               c.outdated = true;
         }

         if (commentData.contains("original_line"))
            c.diff.originalLine = commentData["original_line"].toInt();
         else
            c.diff.originalLine = commentData["original_position"].toInt();

         c.reviewId = commentData["pull_request_review_id"].toInt();
         c.replyToId = commentData["in_reply_to_id"].toInt();

         GitServer::User sAssignee;
         sAssignee.id = commentData["user"].toObject()["id"].toInt();
         sAssignee.url = commentData["user"].toObject()["html_url"].toString();
         sAssignee.name = commentData["user"].toObject()["login"].toString();
         sAssignee.avatar = commentData["user"].toObject()["avatar_url"].toString();
         sAssignee.type = commentData["user"].toObject()["type"].toString();

         c.creator = std::move(sAssignee);

         emit codeReviewsReceived(prNumber, { c });
      }
   });
}

void GitHubRestApi::replyCodeReview(int prNumber, int commentId, const QString &msgBody)
{
   QJsonObject object;
   object.insert("body", msgBody);

   QJsonDocument doc(object);
   const auto data = doc.toJson(QJsonDocument::Compact);

   const auto url = QString("%1/pulls/%2/comments/%3/replies")
                        .arg(mRepoEndpoint, QString::number(prNumber), QString::number(commentId));
   auto request = createRequest(url);
   request.setRawHeader("Content-Length", QByteArray::number(data.size()));
   request.setRawHeader("Accept", "application/vnd.github.v3+json");
   const auto reply = mManager->post(request, data);

   connect(reply, &QNetworkReply::finished, this, [this, prNumber]() {
      const auto reply = qobject_cast<QNetworkReply *>(sender());
      const auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
      QString errorStr;
      const auto tmpDoc = validateData(reply, errorStr);

      if (statusCode.isValid() && statusCode.toInt() == 201 && !tmpDoc.isEmpty())
      {
         const auto commentData = tmpDoc.object();
         CodeReview c;
         c.outdated = false;
         c.id = commentData["id"].toInt();
         c.body = commentData["body"].toString();
         c.creation = commentData["created_at"].toVariant().toDateTime();
         c.association = commentData["author_association"].toString();
         c.diff.diff = commentData["diff_hunk"].toString();
         c.diff.file = commentData["path"].toString();

         if (commentData.contains("line"))
            c.diff.line = commentData["line"].toInt();
         else
         {
            if (commentData["position"].toInt() != 0)
               c.diff.line = commentData["position"].toInt();
            else
               c.outdated = true;
         }

         if (commentData.contains("original_line"))
            c.diff.originalLine = commentData["original_line"].toInt();
         else
            c.diff.originalLine = commentData["original_position"].toInt();

         c.reviewId = commentData["pull_request_review_id"].toInt();
         c.replyToId = commentData["in_reply_to_id"].toInt();

         GitServer::User sAssignee;
         sAssignee.id = commentData["user"].toObject()["id"].toInt();
         sAssignee.url = commentData["user"].toObject()["html_url"].toString();
         sAssignee.name = commentData["user"].toObject()["login"].toString();
         sAssignee.avatar = commentData["user"].toObject()["avatar_url"].toString();
         sAssignee.type = commentData["user"].toObject()["type"].toString();

         c.creator = std::move(sAssignee);

         emit codeReviewsReceived(prNumber, { c });
      }
   });
}

QNetworkRequest GitHubRestApi::createRequest(const QString &page) const
{
   QNetworkRequest request;
   request.setUrl(QUrl(QString("%1%2").arg(mAuth.endpointUrl, page)));
   request.setRawHeader("User-Agent", "GitQlient");
   request.setRawHeader("X-Custom-User-Agent", "GitQlient");
   request.setRawHeader("Content-Type", "application/json");
   request.setRawHeader("Accept", "application/vnd.github.v3+json");
   request.setRawHeader("Authorization", mAuthString);

   return request;
}

void GitHubRestApi::onLabelsReceived()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);
   QVector<Label> labels;

   if (!tmpDoc.isEmpty())
   {
      const auto labelsArray = tmpDoc.array();

      for (const auto &label : labelsArray)
      {
         const auto jobObject = label.toObject();
         Label sLabel { jobObject[QStringLiteral("id")].toInt(),
                        jobObject[QStringLiteral("node_id")].toString(),
                        jobObject[QStringLiteral("url")].toString(),
                        jobObject[QStringLiteral("name")].toString(),
                        jobObject[QStringLiteral("description")].toString(),
                        jobObject[QStringLiteral("color")].toString(),
                        jobObject[QStringLiteral("default")].toBool() };

         labels.append(std::move(sLabel));
      }
   }
   else
      emit errorOccurred(errorStr);

   emit labelsReceived(labels);
}

void GitHubRestApi::onMilestonesReceived()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);
   QVector<Milestone> milestones;

   if (!tmpDoc.isEmpty())
   {
      const auto labelsArray = tmpDoc.array();

      for (const auto &label : labelsArray)
      {
         const auto jobObject = label.toObject();
         Milestone sMilestone { jobObject[QStringLiteral("id")].toInt(),
                                jobObject[QStringLiteral("number")].toInt(),
                                jobObject[QStringLiteral("node_id")].toString(),
                                jobObject[QStringLiteral("title")].toString(),
                                jobObject[QStringLiteral("description")].toString(),
                                jobObject[QStringLiteral("state")].toString() == "open" };

         milestones.append(std::move(sMilestone));
      }
   }
   else
      emit errorOccurred(errorStr);

   emit milestonesReceived(milestones);
}

void GitHubRestApi::onIssueCreated()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      const auto issue = issueFromJson(tmpDoc.object());
      emit issueUpdated(issue);
   }
   else
      emit errorOccurred(errorStr);
}

void GitHubRestApi::onPullRequestCreated()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      const auto pr = prFromJson(tmpDoc.object());

      /*
         QTimer::singleShot(200, [this, number = pr.number]() {
            const auto reply = mManager->get(createRequest(mRepoEndpoint + QString("/pulls/%1").arg(number)));
            connect(reply, &QNetworkReply::finished, this, [this, pr]() { onPullRequestDetailesReceived(pr); });
         });
         */
      QTimer::singleShot(200, this, [this, pr]() {
         auto request = createRequest(mRepoEndpoint + QString("/commits/%1/status").arg(pr.state.sha));
         const auto reply = mManager->get(request);
         connect(reply, &QNetworkReply::finished, this, [this, pr] { onPullRequestStatusReceived(pr); });
      });

      emit pullRequestUpdated(pr);
   }
   else
      emit errorOccurred(errorStr);
}

void GitHubRestApi::onPullRequestMerged()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
      emit pullRequestMerged();
   else
      emit errorOccurred(errorStr);
}

void GitHubRestApi::onPullRequestReceived()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());

   if (const auto pagination = QString::fromUtf8(reply->rawHeader("Link")); !pagination.isEmpty())
   {
      QStringList pages = pagination.split(",");
      auto current = 0;
      auto next = 0;
      auto total = 0;

      for (const auto &page : pages)
      {
         const auto values = page.trimmed().remove("<").remove(">").split(";");

         if (values.last().contains("next"))
         {
            next = values.first().split("page=").last().toInt();
            current = next - 1;
         }
         else if (values.last().contains("last"))
            total = values.first().split("page=").last().toInt();
      }

      emit paginationPresent(current, next, total);
   }
   else
      emit paginationPresent(0, 0, 0);

   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);
   QVector<PullRequest> pullRequests;

   if (!tmpDoc.isEmpty())
   {
      const auto issuesArray = tmpDoc.array();
      for (const auto &issueData : issuesArray)
      {
         const auto pr = prFromJson(issueData.toObject());
         pullRequests.append(pr);

         /*
         QTimer::singleShot(200, [this, number = pr.number]() {
            const auto reply = mManager->get(createRequest(mRepoEndpoint + QString("/pulls/%1").arg(number)));
            connect(reply, &QNetworkReply::finished, this, [this, pr]() { onPullRequestDetailsReceived(pr); });
         });
         */
         QTimer::singleShot(200, this, [this, pr]() {
            auto request = createRequest(mRepoEndpoint + QString("/commits/%1/status").arg(pr.state.sha));
            const auto reply = mManager->get(request);
            connect(reply, &QNetworkReply::finished, this, [this, pr] { onPullRequestStatusReceived(pr); });
         });
      }
   }
   else
      emit errorOccurred(errorStr);

   std::sort(pullRequests.begin(), pullRequests.end(),
             [](const PullRequest &p1, const PullRequest &p2) { return p1.creation > p2.creation; });

   emit pullRequestsReceived(pullRequests);
}

void GitHubRestApi::onPullRequestStatusReceived(PullRequest pr)
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      const auto obj = tmpDoc.object();

      pr.state.state = obj["state"].toString();

      pr.state.eState = pr.state.state == "success" ? PullRequest::HeadState::State::Success
          : pr.state.state == "failure"             ? PullRequest::HeadState::State::Failure
                                                    : PullRequest::HeadState::State::Pending;

      const auto statuses = obj["statuses"].toArray();

      for (const auto &status : statuses)
      {
         auto statusStr = status["state"].toString();

         if (statusStr == "ok")
            statusStr = "success";
         else if (statusStr == "error")
            statusStr = "failure";

         PullRequest::HeadState::Check check { status["description"].toString(), statusStr,
                                               status["target_url"].toString(), status["context"].toString() };

         pr.state.checks.append(std::move(check));
      }

      emit pullRequestUpdated(pr);
   }
   else
      emit errorOccurred(errorStr);
}

void GitHubRestApi::onIssuesReceived()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());

   if (const auto pagination = QString::fromUtf8(reply->rawHeader("Link")); !pagination.isEmpty())
   {
      QStringList pages = pagination.split(",");
      auto current = 0;
      auto next = 0;
      auto total = 0;

      for (const auto &page : pages)
      {
         const auto values = page.trimmed().remove("<").remove(">").split(";");

         if (values.last().contains("next"))
         {
            next = values.first().split("page=").last().toInt();
            current = next - 1;
         }
         else if (values.last().contains("last"))
            total = values.first().split("page=").last().toInt();
      }

      emit paginationPresent(current, next, total);
   }
   else
      emit paginationPresent(0, 0, 0);

   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);
   QVector<Issue> issues;

   if (!tmpDoc.isEmpty())
   {
      const auto issuesArray = tmpDoc.array();

      for (const auto &issueData : issuesArray)
      {
         if (const auto issueObj = issueData.toObject(); !issueObj.contains("pull_request"))
            issues.append(issueFromJson(issueObj));
      }
   }
   else
      emit errorOccurred(errorStr);

   emit issuesReceived(issues);

   for (auto &issue : issues)
      QTimer::singleShot(200, this, [this, num = issue.number]() { requestComments(num); });
}

void GitHubRestApi::onCommentsReceived(int issueNumber)
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      QVector<Comment> comments;
      const auto commentsArray = tmpDoc.array();

      for (const auto &commentData : commentsArray)
      {
         Comment c;
         c.id = commentData["id"].toInt();
         c.body = commentData["body"].toString();
         c.creation = commentData["created_at"].toVariant().toDateTime();
         c.association = commentData["author_association"].toString();

         GitServer::User sAssignee;
         sAssignee.id = commentData["user"].toObject()["id"].toInt();
         sAssignee.url = commentData["user"].toObject()["html_url"].toString();
         sAssignee.name = commentData["user"].toObject()["login"].toString();
         sAssignee.avatar = commentData["user"].toObject()["avatar_url"].toString();
         sAssignee.type = commentData["user"].toObject()["type"].toString();

         c.creator = std::move(sAssignee);
         comments.append(std::move(c));
      }

      emit commentsReceived(issueNumber, comments);
   }
}

void GitHubRestApi::onPullRequestDetailsReceived(PullRequest pr)
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      const auto prInfo = tmpDoc.object();

      pr.commentsCount = prInfo["comments"].toInt();
      pr.reviewCommentsCount = prInfo["review_comments"].toInt();
      pr.commitCount = prInfo["commits"].toInt();
      pr.additions = prInfo["aditions"].toInt();
      pr.deletions = prInfo["deletions"].toInt();
      pr.changedFiles = prInfo["changed_files"].toInt();
      pr.merged = prInfo["merged"].toBool();
      pr.mergeable = prInfo["mergeable"].toBool();
      pr.rebaseable = prInfo["rebaseable"].toBool();
      pr.mergeableState = prInfo["mergeable_state"].toString();

      emit pullRequestUpdated(pr);
   }
}

void GitHubRestApi::onReviewsReceived(int prNumber)
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      QMap<int, Review> reviews;
      const auto commentsArray = tmpDoc.array();

      for (const auto &commentData : commentsArray)
      {
         auto id = commentData["id"].toInt();

         Review r;
         r.id = id;
         r.body = commentData["body"].toString();
         r.creation = commentData["submitted_at"].toVariant().toDateTime();
         r.state = commentData["state"].toString();
         r.association = commentData["author_association"].toString();

         GitServer::User sAssignee;
         sAssignee.id = commentData["user"].toObject()["id"].toInt();
         sAssignee.url = commentData["user"].toObject()["html_url"].toString();
         sAssignee.name = commentData["user"].toObject()["login"].toString();
         sAssignee.avatar = commentData["user"].toObject()["avatar_url"].toString();
         sAssignee.type = commentData["user"].toObject()["type"].toString();

         r.creator = std::move(sAssignee);
         reviews.insert(id, std::move(r));
      }

      emit commentReviewsReceived(prNumber, reviews);

      QTimer::singleShot(200, this, [this, prNumber]() { requestReviewComments(prNumber); });
   }
}

void GitHubRestApi::requestReviewComments(int prNumber)
{
   const auto reply = mManager->get(createRequest(mRepoEndpoint + QString("/pulls/%1/comments").arg(prNumber)));

   connect(reply, &QNetworkReply::finished, this, [this, prNumber]() { onReviewCommentsReceived(prNumber); });
}

void GitHubRestApi::onReviewCommentsReceived(int prNumber)
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      QVector<CodeReview> comments;
      const auto commentsArray = tmpDoc.array();

      for (const auto &commentData : commentsArray)
      {
         CodeReview c;
         c.outdated = false;
         c.id = commentData["id"].toInt();
         c.body = commentData["body"].toString();
         c.creation = commentData["created_at"].toVariant().toDateTime();
         c.association = commentData["author_association"].toString();
         c.diff.diff = commentData["diff_hunk"].toString();
         c.diff.file = commentData["path"].toString();

         if (commentData.toObject().contains("line"))
            c.diff.line = commentData["line"].toInt();
         else
         {
            if (commentData["position"].toInt() != 0)
               c.diff.line = commentData["position"].toInt();
            else
               c.outdated = true;
         }

         if (commentData.toObject().contains("original_line"))
            c.diff.originalLine = commentData["original_line"].toInt();
         else
            c.diff.originalLine = commentData["original_position"].toInt();

         c.reviewId = commentData["pull_request_review_id"].toInt();
         c.replyToId = commentData["in_reply_to_id"].toInt();

         GitServer::User sAssignee;
         sAssignee.id = commentData["user"].toObject()["id"].toInt();
         sAssignee.url = commentData["user"].toObject()["html_url"].toString();
         sAssignee.name = commentData["user"].toObject()["login"].toString();
         sAssignee.avatar = commentData["user"].toObject()["avatar_url"].toString();
         sAssignee.type = commentData["user"].toObject()["type"].toString();

         c.creator = std::move(sAssignee);
         comments.append(std::move(c));
      }

      emit codeReviewsReceived(prNumber, comments);
   }
}

void GitHubRestApi::onCommitsReceived(int prNumber)
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      QVector<Commit> commits;
      const auto commitsArray = tmpDoc.array();

      for (const auto &commitData : commitsArray)
      {
         Commit c;
         c.url = commitData["html_url"].toString();
         c.sha = commitData["sha"].toString();

         GitServer::User sAuthor;
         sAuthor.id = commitData["author"].toObject()["id"].toInt();
         sAuthor.url = commitData["author"].toObject()["html_url"].toString();
         sAuthor.name = commitData["author"].toObject()["login"].toString();
         sAuthor.avatar = commitData["author"].toObject()["avatar_url"].toString();
         sAuthor.type = commitData["author"].toObject()["type"].toString();

         c.author = std::move(sAuthor);

         GitServer::User sCommitter;
         sCommitter.id = commitData["committer"].toObject()["id"].toInt();
         sCommitter.url = commitData["committer"].toObject()["html_url"].toString();
         sCommitter.name = commitData["committer"].toObject()["login"].toString();
         sCommitter.avatar = commitData["committer"].toObject()["avatar_url"].toString();
         sCommitter.type = commitData["committer"].toObject()["type"].toString();

         c.commiter = std::move(sCommitter);

         c.message = commitData["commit"].toObject()["message"].toString();
         c.authorCommittedTimestamp
             = commitData["commit"].toObject()["author"].toObject()["date"].toVariant().toDateTime();

         commits.append(std::move(c));
      }

      const auto link = reply->rawHeader("Link").split(',');

      QString nextUrl;
      QString lastUrl;
      auto currentPage = 0;
      auto lastPage = 0;

      for (auto &pair : link)
      {
         const auto page = pair.split(';');
         const auto rel = page.last().trimmed();

         if (rel.contains("next"))
         {
            nextUrl = QString::fromUtf8(page.first().trimmed());
            nextUrl.remove(0, 1);
            nextUrl.remove(nextUrl.size() - 1, 1);

            currentPage = nextUrl.split("page=").last().toInt();
         }
         else if (rel.contains("last"))
         {
            lastUrl = QString::fromUtf8(page.first().trimmed());
            lastUrl.remove(0, 1);
            lastUrl.remove(lastUrl.size() - 1, 1);

            lastPage = lastUrl.split("page=").last().toInt();
         }
      }

      if (currentPage <= lastPage)
      {
         auto request = createRequest(mRepoEndpoint + QString("/pulls/%1/commits").arg(prNumber));
         request.setUrl(nextUrl);
         const auto reply = mManager->get(request);

         connect(reply, &QNetworkReply::finished, this, [this, prNumber]() { onCommitsReceived(prNumber); });
      }

      std::sort(commits.begin(), commits.end(), [](const Commit &c1, const Commit &c2) {
         return c1.authorCommittedTimestamp < c2.authorCommittedTimestamp;
      });

      emit commitsReceived(prNumber, commits, currentPage, lastPage);
   }
}

Issue GitHubRestApi::issueFromJson(const QJsonObject &json) const
{
   Issue issue;
   issue.number = json["number"].toInt();
   issue.title = json["title"].toString();
   issue.body = json["body"].toString().toUtf8();
   issue.url = json["html_url"].toString();
   issue.creation = json["created_at"].toVariant().toDateTime();
   issue.commentsCount = json["comments"].toInt();
   issue.isOpen = json["state"].toString() == "open";

   issue.creator = { json["user"].toObject()["id"].toInt(), json["user"].toObject()["login"].toString(),
                     json["user"].toObject()["avatar_url"].toString(), json["user"].toObject()["html_url"].toString(),
                     json["user"].toObject()["type"].toString() };

   const auto labels = json["labels"].toArray();

   for (const auto &label : labels)
   {
      issue.labels.append({ label["id"].toInt(), label["node_id"].toString(), label["url"].toString(),
                            label["name"].toString(), label["description"].toString(), label["color"].toString(),
                            label["default"].toBool() });
   }

   const auto assignees = json["assignees"].toArray();

   for (const auto &assignee : assignees)
   {
      GitServer::User sAssignee;
      sAssignee.id = assignee["id"].toInt();
      sAssignee.url = assignee["html_url"].toString();
      sAssignee.name = assignee["login"].toString();
      sAssignee.avatar = assignee["avatar_url"].toString();

      issue.assignees.append(sAssignee);
   }

   if (const auto value = json["milestone"].toString(); !json["milestone"].toObject().isEmpty() && value != "NULL")
   {
      Milestone sMilestone { json["milestone"].toObject()[QStringLiteral("id")].toInt(),
                             json["milestone"].toObject()[QStringLiteral("number")].toInt(),
                             json["milestone"].toObject()[QStringLiteral("node_id")].toString(),
                             json["milestone"].toObject()[QStringLiteral("title")].toString(),
                             json["milestone"].toObject()[QStringLiteral("description")].toString(),
                             json["milestone"].toObject()[QStringLiteral("state")].toString() == "open" };

      issue.milestone = sMilestone;
   }

   return issue;
}

PullRequest GitHubRestApi::prFromJson(const QJsonObject &json) const
{
   PullRequest pr;
   pr.number = json["number"].toInt();
   pr.title = json["title"].toString();
   pr.body = json["body"].toString().toUtf8();
   pr.url = json["html_url"].toString();
   pr.head = json["head"].toObject()["ref"].toString();
   pr.headRepo = json["head"].toObject()["repo"].toObject()["full_name"].toString();
   pr.headUrl = json["head"].toObject()["repo"].toObject()["clone_url"].toString();
   pr.state.sha = json["head"].toObject()["sha"].toString();
   pr.base = json["base"].toObject()["ref"].toString();
   pr.baseRepo = json["base"].toObject()["repo"].toObject()["full_name"].toString();
   pr.isOpen = json["state"].toString() == "open";
   pr.draft = json["draft"].toBool();
   pr.creation = json["created_at"].toVariant().toDateTime();

   pr.creator = { json["user"].toObject()["id"].toInt(), json["user"].toObject()["login"].toString(),
                  json["user"].toObject()["avatar_url"].toString(), json["user"].toObject()["html_url"].toString(),
                  json["user"].toObject()["type"].toString() };

   const auto labels = json["labels"].toArray();

   for (const auto &label : labels)
   {
      pr.labels.append({ label["id"].toInt(), label["node_id"].toString(), label["url"].toString(),
                         label["name"].toString(), label["description"].toString(), label["color"].toString(),
                         label["default"].toBool() });
   }

   const auto assignees = json["assignees"].toArray();

   for (const auto &assignee : assignees)
   {
      GitServer::User sAssignee;
      sAssignee.id = assignee["id"].toInt();
      sAssignee.url = assignee["html_url"].toString();
      sAssignee.name = assignee["login"].toString();
      sAssignee.avatar = assignee["avatar_url"].toString();

      pr.assignees.append(sAssignee);
   }

   Milestone sMilestone { json["milestone"].toObject()[QStringLiteral("id")].toInt(),
                          json["milestone"].toObject()[QStringLiteral("number")].toInt(),
                          json["milestone"].toObject()[QStringLiteral("node_id")].toString(),
                          json["milestone"].toObject()[QStringLiteral("title")].toString(),
                          json["milestone"].toObject()[QStringLiteral("description")].toString(),
                          json["milestone"].toObject()[QStringLiteral("state")].toString() == "open" };

   pr.milestone = sMilestone;

   return pr;
}
