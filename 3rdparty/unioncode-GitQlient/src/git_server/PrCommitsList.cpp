#include <PrCommitsList.h>

#include <GitServerCache.h>
#include <GitHubRestApi.h>
#include <GitLabRestApi.h>
#include <CircularPixmap.h>
#include <ButtonLink.hpp>
#include <AvatarHelper.h>

#include <QApplication>
#include <QClipboard>
#include <QToolTip>
#include <QCursor>
#include <QLabel>
#include <QScrollArea>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QNetworkReply>
#include <QDir>
#include <QFile>

PrCommitsList::PrCommitsList(const QSharedPointer<GitServerCache> &gitServerCache, QWidget *parent)
   : QFrame(parent)
   , mGitServerCache(gitServerCache)
   , mManager(new QNetworkAccessManager())
{
   setObjectName("IssuesViewFrame");
}

PrCommitsList::~PrCommitsList()
{
   delete mManager;
}

void PrCommitsList::loadData(int number)
{
   connect(mGitServerCache.get(), &GitServerCache::prUpdated, this, &PrCommitsList::onCommitsReceived,
           Qt::UniqueConnection);

   mPrNumber = number;

   const auto pr = mGitServerCache->getPullRequest(number);

   mGitServerCache->getApi()->requestCommitsFromPR(pr.number);
}

void PrCommitsList::onCommitsReceived(const GitServer::PullRequest &pr)
{
   disconnect(mGitServerCache.get(), &GitServerCache::prUpdated, this, &PrCommitsList::onCommitsReceived);

   if (mPrNumber != pr.number)
      return;

   delete mScroll;

   mPrNumber = pr.number;

   const auto commitsLayout = new QVBoxLayout();
   commitsLayout->setContentsMargins(20, 20, 20, 20);
   commitsLayout->setAlignment(Qt::AlignTop);
   commitsLayout->setSpacing(30);

   const auto mIssuesFrame = new QFrame();
   mIssuesFrame->setObjectName("IssuesViewFrame");
   mIssuesFrame->setLayout(commitsLayout);

   mScroll = new QScrollArea();
   mScroll->setWidgetResizable(true);
   mScroll->setWidget(mIssuesFrame);

   delete layout();

   const auto aLayout = new QVBoxLayout(this);
   aLayout->setContentsMargins(QMargins());
   aLayout->setSpacing(0);
   aLayout->addWidget(mScroll);

   for (auto &commit : pr.commits)
   {
      const auto bubble = createBubbleForComment(commit);
      commitsLayout->addWidget(bubble);
   }

   commitsLayout->addStretch();
}

QFrame *PrCommitsList::createBubbleForComment(const GitServer::Commit &commit)
{
   const auto days = commit.authorCommittedTimestamp.daysTo(QDateTime::currentDateTime());
   const auto whenText = days <= 30
       ? days != 0 ? tr(" %1 days ago").arg(days) : tr(" today")
       : tr(" on %1").arg(commit.authorCommittedTimestamp.date().toString(QLocale().dateFormat(QLocale::ShortFormat)));

   const auto creator = new QLabel(tr("Committed by <b>%1</b> %2").arg(commit.author.name, whenText));

   auto commitMsg = commit.message.split("\n\n").constFirst().trimmed();

   if (commitMsg.count() >= 47)
      commitMsg = commitMsg.left(47).append("...");

   const auto link = new ButtonLink(QString("<b>%1</b>").arg(commitMsg));
   connect(link, &ButtonLink::clicked, this, [this, sha = commit.sha]() { emit openDiff(sha); });

   const auto frame = new QFrame();
   frame->setObjectName("IssueIntro");

   const auto layout = new QGridLayout(frame);
   layout->setAlignment(Qt::AlignVertical_Mask);
   layout->setContentsMargins(10, 10, 10, 10);
   layout->setHorizontalSpacing(10);
   layout->setVerticalSpacing(5);
   layout->addWidget(createAvatar(commit.author.name, commit.author.avatar), 0, 0, 2, 1);
   layout->addWidget(link, 0, 1);
   layout->addWidget(creator, 1, 1);
   layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 2);

   const auto shaLink = new ButtonLink(commit.sha);
   connect(shaLink, &ButtonLink::clicked, this, [this, sha = commit.sha]() {
      QApplication::clipboard()->setText(sha);
      QToolTip::showText(QCursor::pos(), tr("Copied!"), this);
   });

   layout->addWidget(shaLink, 0, 3, 3, 1);

   if (commit.author.name != commit.commiter.name)
      layout->addWidget(createAvatar(commit.commiter.name, commit.commiter.avatar));

   return frame;
}
