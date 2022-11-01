#include "GitLabRestApi.h"
#include <GitQlientSettings.h>
#include <Issue.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>

using namespace GitServer;

GitLabRestApi::GitLabRestApi(const QString &userName, const QString &repoName, const QString &settingsKey,
                             const ServerAuthentication &auth, QObject *parent)
   : IRestApi(auth, parent)
   , mUserName(userName)
   , mRepoName(repoName)
   , mSettingsKey(settingsKey)
{
   if (!userName.isEmpty() && !auth.userName.isEmpty() && !auth.userPass.isEmpty() && !auth.endpointUrl.isEmpty())
   {
      mPreRequisites = 0;
      GitQlientSettings settings("");
      mUserId = settings.globalValue(QString("%1/%2-userId").arg(mSettingsKey, mRepoName), "").toString();
      mRepoId = settings.globalValue(QString("%1/%2-repoId").arg(mSettingsKey, mRepoName), "").toString();

      if (mRepoId.isEmpty())
      {
         ++mPreRequisites;
         getProjects();
      }

      if (mUserId.isEmpty())
      {
         ++mPreRequisites;
         getUserInfo();
      }
   }
}

void GitLabRestApi::testConnection()
{
   if (mPreRequisites == 0)
   {
      auto request = createRequest("/users");
      auto url = request.url();

      QUrlQuery query;
      query.addQueryItem("username", mUserName);
      url.setQuery(query);
      request.setUrl(url);

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
   else
      mTestRequested = true;
}

void GitLabRestApi::createIssue(const Issue &issue)
{
   auto request = createRequest(QString("/projects/%1/issues").arg(mRepoId));
   auto url = request.url();

   QUrlQuery query;
   query.addQueryItem("title", issue.title);
   query.addQueryItem("description", QString::fromUtf8(issue.body));

   if (!issue.assignees.isEmpty())
      query.addQueryItem("assignee_ids", mUserId);

   if (issue.milestone.id != -1)
      query.addQueryItem("milestone_id", QString::number(issue.milestone.id));

   if (!issue.labels.isEmpty())
   {
      QStringList labelsList;

      for (auto &label : issue.labels)
         labelsList.append(label.name);

      query.addQueryItem("labels", labelsList.join(","));
   }

   url.setQuery(query);
   request.setUrl(url);

   const auto reply = mManager->post(request, "");

   connect(reply, &QNetworkReply::finished, this, &GitLabRestApi::onIssueCreated);
}

void GitLabRestApi::updateIssue(int, const Issue &) { }

void GitLabRestApi::createPullRequest(const PullRequest &pr)
{
   auto request = createRequest(QString("/projects/%1/merge_requests").arg(mRepoId));
   auto url = request.url();

   QUrlQuery query;
   query.addQueryItem("title", pr.title);
   query.addQueryItem("description", QString::fromUtf8(pr.body));
   query.addQueryItem("assignee_ids", mUserId);
   query.addQueryItem("target_branch", pr.base);
   query.addQueryItem("source_branch", pr.head);
   query.addQueryItem("allow_collaboration", QVariant(pr.maintainerCanModify).toString());

   if (pr.milestone.id != -1)
      query.addQueryItem("milestone_id", QString::number(pr.milestone.id));

   if (!pr.labels.isEmpty())
   {
      QStringList labelsList;

      for (auto &label : pr.labels)
         labelsList.append(label.name);

      query.addQueryItem("labels", labelsList.join(","));
   }

   url.setQuery(query);
   request.setUrl(url);

   const auto reply = mManager->post(request, "");

   connect(reply, &QNetworkReply::finished, this, &GitLabRestApi::onMergeRequestCreated);
}

void GitLabRestApi::requestLabels()
{
   const auto reply = mManager->get(createRequest(QString("/projects/%1/labels").arg(mRepoId)));

   connect(reply, &QNetworkReply::finished, this, &GitLabRestApi::onLabelsReceived);
}

void GitLabRestApi::requestMilestones()
{
   const auto reply = mManager->get(createRequest(QString("/projects/%1/milestones").arg(mRepoId)));

   connect(reply, &QNetworkReply::finished, this, &GitLabRestApi::onMilestonesReceived);
}

void GitLabRestApi::requestIssues(int)
{
   auto request = createRequest(QString("/projects/%1/issues").arg(mRepoId));
   auto url = request.url();

   QUrlQuery query;
   query.addQueryItem("state", "opened");
   url.setQuery(query);
   request.setUrl(url);

   const auto reply = mManager->get(request);
   connect(reply, &QNetworkReply::finished, this, &GitLabRestApi::onIssueReceived);
}

void GitLabRestApi::requestPullRequests(int) { }

void GitLabRestApi::onIssueReceived()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
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
}

QNetworkRequest GitLabRestApi::createRequest(const QString &page) const
{
   QNetworkRequest request;
   request.setUrl(QString(mAuth.endpointUrl + page));
   request.setRawHeader("User-Agent", "GitQlient");
   request.setRawHeader("X-Custom-User-Agent", "GitQlient");
   request.setRawHeader("Content-Type", "application/json");
   request.setRawHeader(QByteArray("PRIVATE-TOKEN"),
                        QByteArray(QString(QStringLiteral("%1")).arg(mAuth.userPass).toLocal8Bit()));

   return request;
}

void GitLabRestApi::getUserInfo() const
{
   auto request = createRequest("/users");
   auto url = request.url();

   QUrlQuery query;
   query.addQueryItem("username", mUserName);
   url.setQuery(query);
   request.setUrl(url);

   const auto reply = mManager->get(request);

   connect(reply, &QNetworkReply::finished, this, &GitLabRestApi::onUserInfoReceived, Qt::DirectConnection);
}

void GitLabRestApi::onUserInfoReceived()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      const auto list = tmpDoc.toVariant().toList();

      if (!list.isEmpty())
      {
         const auto firstUser = list.first().toMap();

         mUserId = firstUser.value("id").toString();

         GitQlientSettings settings("");
         settings.setGlobalValue(QString("%1/%2-userId").arg(mSettingsKey, mRepoName), mUserId);

         --mPreRequisites;

         if (mPreRequisites == 0 && mTestRequested)
            testConnection();
      }
   }
   else
      emit errorOccurred(errorStr);
}

void GitLabRestApi::getProjects()
{
   auto request = createRequest(QString("/users/%1/projects").arg(mUserName));
   const auto reply = mManager->get(request);

   connect(reply, &QNetworkReply::finished, this, &GitLabRestApi::onProjectsReceived, Qt::DirectConnection);
}

void GitLabRestApi::onProjectsReceived()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      const auto projectsObj = tmpDoc.toVariant().toList();

      for (const auto &projObj : projectsObj)
      {
         const auto labelMap = projObj.toMap();

         if (labelMap.value("path").toString() == mRepoName)
         {
            mRepoId = labelMap.value("id").toString();

            GitQlientSettings settings("");
            settings.setGlobalValue(QString("%1/%2-repoId").arg(mSettingsKey, mRepoName), mRepoId);
            --mPreRequisites;

            if (mPreRequisites == 0 && mTestRequested)
               testConnection();

            break;
         }
      }
   }
   else
      emit errorOccurred(errorStr);
}

void GitLabRestApi::onLabelsReceived()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);
   QVector<Label> labels;

   if (!tmpDoc.isEmpty())
   {
      const auto labelsObj = tmpDoc.toVariant().toList();

      for (const auto &labelObj : labelsObj)
      {
         const auto labelMap = labelObj.toMap();
         Label sLabel { labelMap.value("id").toString().toInt(),
                        "",
                        "",
                        labelMap.value("name").toString(),
                        labelMap.value("description").toString(),
                        labelMap.value("color").toString(),
                        false };

         labels.append(std::move(sLabel));
      }
   }
   else
      emit errorOccurred(errorStr);

   emit labelsReceived(labels);
}

void GitLabRestApi::onMilestonesReceived()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      const auto milestonesObj = tmpDoc.toVariant().toList();

      QVector<Milestone> milestones;

      for (const auto &milestoneObj : milestonesObj)
      {
         const auto labelMap = milestoneObj.toMap();
         Milestone sMilestone {
            labelMap.value("id").toString().toInt(),  labelMap.value("id").toString().toInt(),
            labelMap.value("iid").toString(),         labelMap.value("title").toString(),
            labelMap.value("description").toString(), labelMap.value("state").toString() == "active"
         };

         milestones.append(std::move(sMilestone));
      }

      emit milestonesReceived(milestones);
   }
   else
      emit errorOccurred(errorStr);
}

void GitLabRestApi::onIssueCreated()
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

void GitLabRestApi::onMergeRequestCreated()
{
   const auto reply = qobject_cast<QNetworkReply *>(sender());
   QString errorStr;
   const auto tmpDoc = validateData(reply, errorStr);

   if (!tmpDoc.isEmpty())
   {
      const auto issue = prFromJson(tmpDoc.object());
      emit pullRequestUpdated(issue);
   }
   else
      emit errorOccurred(errorStr);
}

Issue GitLabRestApi::issueFromJson(const QJsonObject &json) const
{
   Issue issue;
   issue.number = json["id"].toInt();
   issue.title = json["title"].toString();
   issue.body = json["description"].toString().toUtf8();
   issue.url = json["web_url"].toString();
   issue.creation = json["created_at"].toVariant().toDateTime();
   issue.commentsCount = json["comments"].toInt();

   issue.creator
       = { json["author"].toObject()["id"].toInt(), json["author"].toObject()["username"].toString(),
           json["author"].toObject()["avatar_url"].toString(), json["author"].toObject()["web_url"].toString(), "" };

   const auto labels = json["labels"].toArray();

   for (const auto &label : labels)
   {
      Label sLabel;
      sLabel.name = label.toString();
      issue.labels.append(sLabel);
   }

   const auto assignees = json["assignees"].toArray();

   for (const auto &assignee : assignees)
   {
      GitServer::User sAssignee;
      sAssignee.id = assignee["id"].toInt();
      sAssignee.url = assignee["web_url"].toString();
      sAssignee.name = assignee["username"].toString();
      sAssignee.avatar = assignee["avatar_url"].toString();

      issue.assignees.append(sAssignee);
   }

   Milestone sMilestone { json["milestone"].toObject()[QStringLiteral("id")].toInt(),
                          json["milestone"].toObject()[QStringLiteral("number")].toInt(),
                          json["milestone"].toObject()[QStringLiteral("iid")].toString(),
                          json["milestone"].toObject()[QStringLiteral("title")].toString(),
                          json["milestone"].toObject()[QStringLiteral("description")].toString(),
                          json["milestone"].toObject()[QStringLiteral("state")].toString() == "open" };

   issue.milestone = sMilestone;

   return issue;
}

PullRequest GitLabRestApi::prFromJson(const QJsonObject &json) const
{
   PullRequest pr;
   pr.id = json["id"].toInt();
   pr.number = json["id"].toInt();
   pr.title = json["title"].toString();
   pr.body = json["description"].toString().toUtf8();
   pr.url = json["web_url"].toString();
   pr.head = json["head"].toObject()["ref"].toString();
   pr.state.sha = json["head"].toObject()["sha"].toString();
   pr.base = json["base"].toObject()["ref"].toString();
   pr.isOpen = json["state"].toString() == "open";
   pr.draft = json["draft"].toBool();
   pr.creation = json["created_at"].toVariant().toDateTime();

   pr.creator
       = { json["author"].toObject()["id"].toInt(), json["author"].toObject()["username"].toString(),
           json["author"].toObject()["avatar_url"].toString(), json["author"].toObject()["web_url"].toString(), "" };

   const auto labels = json["labels"].toArray();

   for (const auto &label : labels)
   {
      Label sLabel;
      sLabel.name = label.toString();
      pr.labels.append(sLabel);
   }

   const auto assignee = json["assignee"].toObject();
   GitServer::User sAssignee;
   sAssignee.id = assignee["id"].toInt();
   sAssignee.url = assignee["web_url"].toString();
   sAssignee.name = assignee["username"].toString();
   sAssignee.avatar = assignee["avatar_url"].toString();

   pr.assignees.append(sAssignee);

   Milestone sMilestone { json["milestone"].toObject()[QStringLiteral("id")].toInt(),
                          json["milestone"].toObject()[QStringLiteral("number")].toInt(),
                          json["milestone"].toObject()[QStringLiteral("iid")].toString(),
                          json["milestone"].toObject()[QStringLiteral("title")].toString(),
                          json["milestone"].toObject()[QStringLiteral("description")].toString(),
                          json["milestone"].toObject()[QStringLiteral("state")].toString() == "open" };

   pr.milestone = sMilestone;

   return pr;
}
