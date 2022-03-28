#include <GitServerCache.h>

#include <GitQlientSettings.h>
#include <GitConfig.h>
#include <GitHubRestApi.h>
#include <GitLabRestApi.h>

#include <Label.h>
#include <Milestone.h>

using namespace GitServer;

GitServerCache::GitServerCache(QObject *parent)
   : QObject(parent)
{
}

GitServerCache::~GitServerCache() { }

bool GitServerCache::init(const QString &serverUrl, const QPair<QString, QString> &repoInfo)
{
   mInit = true;

   GitQlientSettings settings;
   const auto userName = settings.globalValue(QString("%1/user").arg(serverUrl)).toString();
   const auto userToken = settings.globalValue(QString("%1/token").arg(serverUrl)).toString();
   const auto endpoint = settings.globalValue(QString("%1/endpoint").arg(serverUrl)).toString();

   if (serverUrl.contains("github"))
      mApi.reset(new GitHubRestApi(repoInfo.first, repoInfo.second, { userName, userToken, endpoint }));
   else if (serverUrl.contains("gitlab"))
      mApi.reset(new GitLabRestApi(userName, repoInfo.second, serverUrl, { userName, userToken, endpoint }));
   else
   {
      mInit = false;
      return mInit;
   }

   connect(getApi(), &IRestApi::labelsReceived, this, &GitServerCache::initLabels);
   connect(getApi(), &IRestApi::milestonesReceived, this, &GitServerCache::initMilestones);
   connect(getApi(), &IRestApi::issuesReceived, this, &GitServerCache::initIssues);
   connect(getApi(), &IRestApi::pullRequestsReceived, this, &GitServerCache::initPullRequests);
   connect(getApi(), &IRestApi::commentsReceived, this, &GitServerCache::onCommentsReceived);
   connect(getApi(), &IRestApi::codeReviewsReceived, this, &GitServerCache::onCodeReviewsReceived);
   connect(getApi(), &IRestApi::commentReviewsReceived, this, &GitServerCache::onCommentReviewsReceived);
   connect(getApi(), &IRestApi::commitsReceived, this, &GitServerCache::onCommitsReceived);
   connect(getApi(), &IRestApi::issueUpdated, this, &GitServerCache::onIssueUpdated);
   connect(getApi(), &IRestApi::pullRequestUpdated, this, &GitServerCache::onPRUpdated);
   connect(getApi(), &IRestApi::errorOccurred, this, &GitServerCache::errorOccurred);
   connect(getApi(), &IRestApi::connectionTested, this, &GitServerCache::onConnectionTested);

   mApi->testConnection();

   mWaitingConfirmation = true;

   return mInit;
}

QString GitServerCache::getUserName() const
{
   return mApi->getUserName();
}

QVector<PullRequest> GitServerCache::getPullRequests() const
{
   auto pullRequests = mPullRequests.values();

   std::sort(pullRequests.begin(), pullRequests.end(),
             [](const PullRequest &p1, const PullRequest &p2) { return p1.creation > p2.creation; });

   return pullRequests.toVector();
}

void GitServerCache::onConnectionTested()
{
   mPreSteps = 3;

   mApi->requestLabels();
   mApi->requestMilestones();
   mApi->requestIssues();
   mApi->requestPullRequests();

   /*
   connect(mApi.get(), &IRestApi::milestonesReceived, this, [](){});
   connect(mApi.get(), &IRestApi::milestonesReceived, this, [](){});
   */
}

void GitServerCache::onIssueUpdated(const Issue &issue)
{
   mIssues[issue.number] = issue;

   emit issueUpdated(issue);
}

void GitServerCache::onPRUpdated(const PullRequest &pr)
{
   mPullRequests[pr.number] = pr;

   emit prUpdated(pr);
}

void GitServerCache::onCommentsReceived(int number, const QVector<Comment> &comments)
{
   if (mIssues.contains(number))
   {
      mIssues[number].comments = comments;
      emit issueUpdated(mIssues[number]);
   }
   else if (mPullRequests.contains(number))
   {
      mPullRequests[number].comments = comments;
      emit prReviewsReceived();
   }
}

void GitServerCache::onCodeReviewsReceived(int number, const QVector<GitServer::CodeReview> &codeReviews)
{
   if (mPullRequests.contains(number))
   {
      for (auto &cReview : codeReviews)
      {
         if (!mPullRequests[number].reviewComment.contains(cReview))
            mPullRequests[number].reviewComment.append(cReview);
      }
   }

   emit prReviewsReceived();
}

void GitServerCache::onCommentReviewsReceived(int number, const QMap<int, GitServer::Review> &commentReviews)
{
   if (mPullRequests.contains(number))
   {
      const auto end = commentReviews.cend();

      for (auto iter = commentReviews.cbegin(); iter != end; ++iter)
         mPullRequests[number].reviews.insert(iter.key(), iter.value());

      emit prReviewsReceived();
   }
}

void GitServerCache::onCommitsReceived(int number, const QVector<GitServer::Commit> &commits, int currentPage,
                                       int lastPage)
{
   if (mPullRequests.contains(number))
   {
      if (currentPage == 1)
         mPullRequests[number].commits.clear();

      for (auto &commit : commits)
      {
         if (!mPullRequests[number].commits.contains(commit))
            mPullRequests[number].commits.append(commit);
      }
   }

   if (currentPage == lastPage)
      emit prUpdated(mPullRequests[number]);
}

PullRequest GitServerCache::getPullRequest(const QString &sha) const
{
   const auto iter = std::find_if(mPullRequests.constBegin(), mPullRequests.constEnd(),
                                  [sha](const GitServer::PullRequest &pr) { return pr.state.sha == sha; });

   if (iter != mPullRequests.constEnd())
      return *iter;

   return PullRequest();
}

QVector<Issue> GitServerCache::getIssues() const
{
   auto issues = mIssues.values();

   std::sort(issues.begin(), issues.end(), [](const Issue &i1, const Issue &i2) { return i1.creation > i2.creation; });

   return issues.toVector();
}

GitServer::Platform GitServerCache::getPlatform() const
{
   if (dynamic_cast<GitHubRestApi *>(getApi()))
      return Platform::GitHub;

   return Platform::GitLab;
}

IRestApi *GitServerCache::getApi() const
{
   return mApi.get();
}

void GitServerCache::initLabels(const QVector<Label> &labels)
{
   mLabels = labels;

   triggerSignalConditionally();
}

void GitServerCache::initMilestones(const QVector<Milestone> &milestones)
{
   mMilestones = milestones;

   triggerSignalConditionally();
}

void GitServerCache::initIssues(const QVector<Issue> &issues)
{
   for (auto &issue : issues)
      mIssues.insert(issue.number, issue);

   triggerSignalConditionally();

   emit issuesReceived();
}

void GitServerCache::initPullRequests(const QVector<PullRequest> &prs)
{
   for (auto &pr : prs)
      mPullRequests.insert(pr.number, pr);

   triggerSignalConditionally();

   emit prReceived();
}

void GitServerCache::triggerSignalConditionally()
{
   --mPreSteps;

   if (mWaitingConfirmation && mPreSteps == 0)
   {
      mWaitingConfirmation = false;
      mPreSteps = -1;
      emit connectionTested();
   }
}
