#include "JenkinsWidget.h"

#include <RepoFetcher.h>
#include <JobContainer.h>
#include <GitQlientSettings.h>
#include <GitBase.h>

#include <QTimer>
#include <QButtonGroup>
#include <QStackedLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QNetworkAccessManager>

namespace Jenkins
{

JenkinsWidget::JenkinsWidget(const QSharedPointer<GitBase> &git, QWidget *parent)
   : QWidget(parent)
   , mGit(git)
   , mStackedLayout(new QStackedLayout())
   , mBodyLayout(new QHBoxLayout())
   , mBtnGroup(new QButtonGroup())
   , mButtonsLayout(new QVBoxLayout())
   , mTimer(new QTimer(this))
{
   setObjectName("JenkinsWidget");

   GitQlientSettings settings(mGit->getGitDir());
   const auto url = settings.localValue("BuildSystemUrl", "").toString();
   const auto user = settings.localValue("BuildSystemUser", "").toString();
   const auto token = settings.localValue("BuildSystemToken", "").toString();

   mConfig = IFetcher::Config { user, token, nullptr };
   mConfig.accessManager.reset(new QNetworkAccessManager());

   const auto superBtnsLayout = new QVBoxLayout();
   superBtnsLayout->setContentsMargins(QMargins());
   superBtnsLayout->setSpacing(0);
   superBtnsLayout->addLayout(mButtonsLayout);
   superBtnsLayout->addStretch();

   mBodyLayout->setSpacing(0);
   mBodyLayout->addLayout(superBtnsLayout);
   mBodyLayout->addLayout(mStackedLayout);

   const auto layout = new QHBoxLayout(this);
   layout->setContentsMargins(QMargins());
   layout->setSpacing(0);
   layout->addLayout(mBodyLayout);

   setMinimumSize(800, 600);

   mRepoFetcher = new RepoFetcher(mConfig, url, this);
   connect(mRepoFetcher, &RepoFetcher::signalViewsReceived, this, &JenkinsWidget::configureGeneralView);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
   connect(mBtnGroup, &QButtonGroup::idClicked, mStackedLayout, &QStackedLayout::setCurrentIndex);
#else
   connect(mBtnGroup, SIGNAL(buttonClicked(int)), mStackedLayout, SLOT(setCurrentIndex(int)));
#endif

   mTimer->setInterval(15 * 60 * 1000); // 15 mins
}

JenkinsWidget::~JenkinsWidget()
{
   delete mBtnGroup;
}

void JenkinsWidget::reload() const
{
   mTimer->stop();
   mRepoFetcher->triggerFetch();
   mTimer->start();
}

void JenkinsWidget::configureGeneralView(const QVector<JenkinsViewInfo> &views)
{
   for (auto &view : views)
   {
      if (!mViews.contains(view))
      {
         const auto button = new QPushButton(view.name);
         button->setObjectName("JenkinsWidgetTabButton");
         button->setCheckable(true);

         const auto container = new JobContainer(mConfig, view, this);
         container->setObjectName("JobContainer");
         connect(container, &JobContainer::signalJobAreViews, this, &JenkinsWidget::configureGeneralView);
         connect(container, &JobContainer::gotoBranch, this, &JenkinsWidget::gotoBranch);
         connect(container, &JobContainer::gotoPullRequest, this, &JenkinsWidget::gotoPullRequest);

         mJobsMap.insert(view.name, container);

         mButtonsLayout->addWidget(button);
         const auto id = mStackedLayout->addWidget(container);
         mBtnGroup->addButton(button, id);

         mViews.append(view);

         if (mViews.count() == 1)
            button->setChecked(true);
      }
      else
         mJobsMap[view.name]->reload();
   }
}

}
