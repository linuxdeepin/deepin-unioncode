#include <AGitServerItemList.h>

#include <GitServerCache.h>
#include <IssueItem.h>
#include <ClickableFrame.h>
#include <IRestApi.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QSpinBox>
#include <QToolButton>

using namespace GitServer;

AGitServerItemList::AGitServerItemList(const QSharedPointer<GitServerCache> &gitServerCache, QWidget *parent)
   : QFrame(parent)
   , mGitServerCache(gitServerCache)
   , mHeaderTitle(new QLabel())
   , mArrow(new QLabel())
{
   mHeaderTitle->setObjectName("HeaderTitle");

   const auto headerFrame = new ClickableFrame();
   headerFrame->setObjectName("IssuesHeaderFrame");
   connect(headerFrame, &ClickableFrame::clicked, this, &AGitServerItemList::onHeaderClicked);

   mArrow->setPixmap(QIcon(":/icons/remove").pixmap(QSize(15, 15)));

   const auto headerLayout = new QHBoxLayout(headerFrame);
   headerLayout->setContentsMargins(QMargins());
   headerLayout->setSpacing(0);
   headerLayout->addWidget(mHeaderTitle);
   headerLayout->addStretch();
   headerLayout->addWidget(mArrow);

   mIssuesLayout = new QVBoxLayout();

   const auto issuesLayout = new QVBoxLayout(this);
   issuesLayout->setContentsMargins(QMargins());
   issuesLayout->setSpacing(0);
   issuesLayout->setAlignment(Qt::AlignTop);
   issuesLayout->addWidget(headerFrame);
   issuesLayout->addLayout(mIssuesLayout);

   const auto timer = new QTimer();
   connect(timer, &QTimer::timeout, this, &AGitServerItemList::loadData);
   timer->start(900000);
}

void AGitServerItemList::loadData()
{
   loadPage();
}

void AGitServerItemList::createContent(QVector<IssueItem *> items)
{
   delete mIssuesWidget;
   delete mScrollArea;

   const auto issuesLayout = new QVBoxLayout();
   issuesLayout->setAlignment(Qt::AlignTop | Qt::AlignVCenter);
   issuesLayout->setContentsMargins(QMargins());
   issuesLayout->setSpacing(0);

   for (auto item : items)
   {
      issuesLayout->addWidget(item);

      const auto separator = new QFrame();
      separator->setObjectName("separator");
      issuesLayout->addWidget(separator);
   }

   issuesLayout->addStretch();

   mIssuesWidget = new QFrame();
   mIssuesWidget->setLayout(issuesLayout);
   mIssuesWidget->setObjectName("IssuesWidget");

   mScrollArea = new QScrollArea();
   mScrollArea->setWidget(mIssuesWidget);
   mScrollArea->setWidgetResizable(true);
   mScrollArea->setObjectName("IssuesScroll");
   mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

   mIssuesLayout->addWidget(mScrollArea);

   const auto icon = QIcon(mScrollArea->isVisible() ? QString(":/icons/add") : QString(":/icons/remove"));
   mArrow->setPixmap(icon.pixmap(QSize(15, 15)));

   setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
}

void AGitServerItemList::onHeaderClicked()
{
   if (mScrollArea)
   {
      const auto issuesVisible = mScrollArea->isVisible();

      mScrollArea->setWidgetResizable(issuesVisible);

      const auto icon = QIcon(issuesVisible ? QString(":/icons/add") : QString(":/icons/remove"));
      mArrow->setPixmap(icon.pixmap(QSize(15, 15)));
      mScrollArea->setVisible(!issuesVisible);

      /*
      GitQlientSettings settings;
      settings.setLocalValue(mGit->getGitQlientSettingsDir(),
      QString("IssuesWidgetHeader/%1").arg(static_cast<int>(mConfig), !tagsAreVisible);
      */

      if (issuesVisible)
         setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
      else
         setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
   }
}

void AGitServerItemList::loadPage(int)
{
   /*
   if (page == -1)
   {
      if (mConfig == Config::Issues)
         mApi->requestIssues();
      else
         mApi->requestPullRequests();
   }
   else
   {
      if (mConfig == Config::Issues)
         mApi->requestIssues(page);
      else
         mApi->requestPullRequests(page);
   }
*/
}
