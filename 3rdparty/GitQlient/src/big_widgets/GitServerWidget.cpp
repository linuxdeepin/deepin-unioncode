#include <GitServerWidget.h>

#include <ServerConfigDlg.h>
#include <GitServerCache.h>
//#include <CreateIssueDlg.h>
//#include <CreatePullRequestDlg.h>
#include <GitHubRestApi.h>
#include <GitLabRestApi.h>
#include <IssuesList.h>
#include <PrList.h>
#include <IssueDetailedView.h>
#include <Platform.h>
#include <GitBase.h>

#include <QPushButton>
#include <QToolButton>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QStackedLayout>
#include <QLabel>
#include <QMessageBox>
#include <QStackedLayout>

using namespace GitServer;

GitServerWidget::GitServerWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                                 const QSharedPointer<GitServerCache> &gitServerCache, QWidget *parent)
   : QFrame(parent)
   , mCache(cache)
   , mGit(git)
   , mGitServerCache(gitServerCache)
{
}

GitServerWidget::~GitServerWidget()
{
   delete mDetailedView;
   delete mOldIssue;
   delete mOldPr;
   delete mRefresh;
}

bool GitServerWidget::configure(const GitServer::ConfigData &config)
{
   if (mConfigured)
      return true;

   if (config.user.isEmpty() || config.token.isEmpty())
   {
      const auto configDlg = new ServerConfigDlg(mGitServerCache, config, this);
      mConfigured = configDlg->exec() == QDialog::Accepted;
   }
   else
      mConfigured = true;

   if (mConfigured)
      createWidget();

   return mConfigured;
}

void GitServerWidget::openPullRequest(int prNumber)
{
   mDetailedView->loadData(IssueDetailedView::Config::PullRequests, prNumber);
}

void GitServerWidget::createWidget()
{
   const auto prLabel = QString::fromUtf8(
       mGitServerCache->getPlatform() == GitServer::Platform::GitHub ? "pull request" : "merge request");

   const auto home = new QPushButton();
   home->setIcon(QIcon(":/icons/home"));
   home->setToolTip(tr("General view"));

   const auto newIssue = new QPushButton();
   newIssue->setIcon(QIcon(":/icons/new_issue"));
   newIssue->setToolTip(tr("Create a new issue"));

   const auto newPr = new QPushButton();
   newPr->setIcon(QIcon(":/icons/new_pr"));
   newPr->setToolTip(tr("Create a new %1").arg(prLabel));

   const auto refresh = new QPushButton();
   refresh->setIcon(QIcon(":/icons/refresh"));
   refresh->setToolTip(tr("Refresh"));

   const auto buttonsLayout = new QHBoxLayout();
   buttonsLayout->setContentsMargins(QMargins());
   buttonsLayout->setSpacing(10);
   buttonsLayout->addWidget(home);
   buttonsLayout->addWidget(newIssue);
   buttonsLayout->addWidget(newPr);
   buttonsLayout->addWidget(refresh);
   buttonsLayout->addStretch();

   mDetailedView = new IssueDetailedView(mGit, mGitServerCache);
   connect(mDetailedView, &IssueDetailedView::openDiff, this, &GitServerWidget::openDiff);

   const auto issues = new IssuesList(mGitServerCache);
   connect(issues, &AGitServerItemList::selected, mDetailedView,
           [this](int issueNum) { mDetailedView->loadData(IssueDetailedView::Config::Issues, issueNum); });

   const auto pullRequests = new PrList(mGitServerCache);
   connect(pullRequests, &AGitServerItemList::selected, this, &GitServerWidget::openPullRequest);

   connect(refresh, &QPushButton::clicked, this, [issues, pullRequests]() {
      issues->refreshData();
      pullRequests->refreshData();
   });

   const auto issuesLayout = new QVBoxLayout();
   issuesLayout->setContentsMargins(QMargins());
   issuesLayout->setSpacing(10);
   issuesLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
   issuesLayout->addWidget(issues);
   issuesLayout->addWidget(pullRequests);

   const auto detailsLayout = new QVBoxLayout();
   detailsLayout->setContentsMargins(QMargins());
   detailsLayout->setSpacing(10);
   detailsLayout->setAlignment(Qt::AlignTop);
   detailsLayout->addWidget(mDetailedView);

   const auto generalViewLayout = new QGridLayout();
   generalViewLayout->setContentsMargins(10, 10, 10, 10);
   generalViewLayout->setSpacing(10);
   generalViewLayout->setColumnStretch(0, 1);
   generalViewLayout->setColumnStretch(1, 3);
   generalViewLayout->addLayout(issuesLayout, 2, 0);
   generalViewLayout->addLayout(detailsLayout, 2, 1);

   mGeneralView = new QFrame();
   mGeneralView->setLayout(generalViewLayout);

//   mCreateIssueView = new CreateIssueDlg(mGitServerCache, mGit->getWorkingDir(), this);
//   mCreatePrView = new CreatePullRequestDlg(mCache, mGitServerCache, this);

   mStackedLayout = new QStackedLayout();
   mStackedLayout->addWidget(mGeneralView);
//   mStackedLayout->addWidget(mCreateIssueView);
//   mStackedLayout->addWidget(mCreatePrView);

   const auto centralLayout = new QVBoxLayout();
   centralLayout->setContentsMargins(10, 10, 10, 10);
   centralLayout->setSpacing(10);
   centralLayout->addLayout(buttonsLayout);
   centralLayout->addLayout(mStackedLayout);

   issues->loadData();
   pullRequests->loadData();

   connect(home, &QPushButton::clicked, this, [this]() { mStackedLayout->setCurrentIndex(0); });
   connect(newIssue, &QPushButton::clicked, [this]() { mStackedLayout->setCurrentIndex(1); });
   connect(newPr, &QPushButton::clicked, [this]() {
      mStackedLayout->setCurrentIndex(2);
//      mCreatePrView->configure(mGit->getWorkingDir(), mGit->getCurrentBranch());
   });

   delete mOldIssue;
   mOldIssue = newIssue;

   delete mOldPr;
   mOldPr = newPr;

   delete mRefresh;
   mRefresh = refresh;

   delete layout();
   setLayout(centralLayout);
}
