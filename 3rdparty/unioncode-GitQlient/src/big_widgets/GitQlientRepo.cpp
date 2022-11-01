#include "GitQlientRepo.h"

#include <BlameWidget.h>
#include <BranchesWidget.h>
#include <CommitHistoryColumns.h>
#include <CommitInfo.h>
#include <ConfigData.h>
#include <ConfigWidget.h>
#include <Controls.h>
#include <DiffWidget.h>
#include <GitBase.h>
#include <GitCache.h>
#include <GitConfig.h>
#include <GitConfigDlg.h>
#include <GitHistory.h>
#include <GitHubRestApi.h>
#include <GitLocal.h>
#include <GitMerge.h>
#include <GitQlientSettings.h>
#include <GitRepoLoader.h>
#include <GitServerCache.h>
#include <GitServerWidget.h>
#include <GitSubmodules.h>
#include <GitTags.h>
#include <GitWip.h>
#include <HistoryWidget.h>
#include <JenkinsWidget.h>
#include <MergeWidget.h>
#include <QLogger.h>
#include <WaitingDlg.h>

#include <QApplication>
#include <QDirIterator>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QTimer>

using namespace QLogger;
using namespace GitServer;
using namespace Jenkins;

GitQlientRepo::GitQlientRepo(const QSharedPointer<GitBase> &git, const QSharedPointer<GitQlientSettings> &settings,
                             QWidget *parent)
   : QFrame(parent)
   , mGitQlientCache(new GitCache())
   , mGitServerCache(new GitServerCache())
   , mGitBase(git)
   , mSettings(settings)
   , mGitLoader(new GitRepoLoader(mGitBase, mGitQlientCache, mSettings))
   , mHistoryWidget(new HistoryWidget(mGitQlientCache, mGitBase, mGitServerCache, mSettings))
   , mStackedLayout(new QStackedLayout())
   , mControls(new Controls(mGitQlientCache, mGitBase))
   , mDiffWidget(new DiffWidget(mGitBase, mGitQlientCache))
   , mBlameWidget(new BlameWidget(mGitQlientCache, mGitBase, mSettings))
   , mMergeWidget(new MergeWidget(mGitQlientCache, mGitBase))
   , mGitServerWidget(new GitServerWidget(mGitQlientCache, mGitBase, mGitServerCache))
   , mJenkins(new JenkinsWidget(mGitBase))
   , mConfigWidget(new ConfigWidget(mGitBase))
   , mAutoFetch(new QTimer())
   , mAutoFilesUpdate(new QTimer())
   , mGitTags(new GitTags(mGitBase, mGitQlientCache))
{
   setAttribute(Qt::WA_DeleteOnClose);

   QLog_Info("UI", QString("Initializing GitQlient"));

   setObjectName("mainWindow");
   setWindowTitle("GitQlient");
   setAttribute(Qt::WA_DeleteOnClose);

   QScopedPointer<GitConfig> gitConfig(new GitConfig(mGitBase));
   const auto serverUrl = gitConfig->getServerHost();
   const auto repoInfo = gitConfig->getCurrentRepoAndOwner();

   mGitServerCache->init(serverUrl, repoInfo);

   mHistoryWidget->setContentsMargins(QMargins(5, 5, 5, 5));
   mDiffWidget->setContentsMargins(QMargins(5, 5, 5, 5));
   mBlameWidget->setContentsMargins(QMargins(5, 5, 5, 5));
   mMergeWidget->setContentsMargins(QMargins(5, 5, 5, 5));
   mGitServerWidget->setContentsMargins(QMargins(5, 5, 5, 5));
   mJenkins->setContentsMargins(QMargins(5, 5, 5, 5));
   mConfigWidget->setContentsMargins(QMargins(5, 5, 5, 5));

   mStackedLayout->addWidget(mHistoryWidget);
   mStackedLayout->addWidget(mDiffWidget);
   mStackedLayout->addWidget(mBlameWidget);
   mStackedLayout->addWidget(mMergeWidget);
   mStackedLayout->addWidget(mGitServerWidget);
   mStackedLayout->addWidget(mJenkins);
   mStackedLayout->addWidget(mConfigWidget);

   const auto mainLayout = new QVBoxLayout();
   mainLayout->setSpacing(0);
   mainLayout->setContentsMargins(QMargins());
   mainLayout->addWidget(mControls);
   mainLayout->addLayout(mStackedLayout);

   setLayout(mainLayout);

   showHistoryView();

   const auto fetchInterval = mSettings->localValue("AutoFetch", 5).toInt();

   mAutoFetch->setInterval(fetchInterval * 60 * 1000);
   mAutoFilesUpdate->setInterval(15000);

   connect(mAutoFetch, &QTimer::timeout, mControls, &Controls::fetchAll);
   connect(mAutoFilesUpdate, &QTimer::timeout, this, &GitQlientRepo::updateUiFromWatcher);

   connect(mControls, &Controls::requestFullReload, this, &GitQlientRepo::fullReload);
   connect(mControls, &Controls::requestReferencesReload, this, &GitQlientRepo::referencesReload);

   connect(mControls, &Controls::signalGoRepo, this, &GitQlientRepo::showHistoryView);
   connect(mControls, &Controls::signalGoBlame, this, &GitQlientRepo::showBlameView);
   connect(mControls, &Controls::signalGoDiff, this, &GitQlientRepo::showDiffView);
   connect(mControls, &Controls::signalGoMerge, this, &GitQlientRepo::showMergeView);
   connect(mControls, &Controls::signalGoServer, this, &GitQlientRepo::showGitServerView);
   connect(mControls, &Controls::signalGoBuildSystem, this, &GitQlientRepo::showBuildSystemView);
   connect(mControls, &Controls::goConfig, this, &GitQlientRepo::showConfig);
   connect(mControls, &Controls::signalPullConflict, mControls, &Controls::activateMergeWarning);
   connect(mControls, &Controls::signalPullConflict, this, &GitQlientRepo::showWarningMerge);

   connect(mHistoryWidget, &HistoryWidget::signalAllBranchesActive, mGitLoader.data(), &GitRepoLoader::setShowAll);
   connect(mHistoryWidget, &HistoryWidget::fullReload, this, &GitQlientRepo::fullReload);
   connect(mHistoryWidget, &HistoryWidget::referencesReload, this, &GitQlientRepo::referencesReload);
   connect(mHistoryWidget, &HistoryWidget::logReload, this, &GitQlientRepo::logReload);

   connect(mHistoryWidget, &HistoryWidget::panelsVisibilityChanged, mConfigWidget,
           &ConfigWidget::onPanelsVisibilityChanged);
   connect(mHistoryWidget, &HistoryWidget::signalOpenSubmodule, this, &GitQlientRepo::signalOpenSubmodule);
   connect(mHistoryWidget, &HistoryWidget::signalOpenDiff, this, &GitQlientRepo::openCommitDiff);
   connect(mHistoryWidget, &HistoryWidget::signalOpenCompareDiff, this, &GitQlientRepo::openCommitCompareDiff);
   connect(mHistoryWidget, &HistoryWidget::signalShowDiff, this, &GitQlientRepo::loadFileDiff);
   connect(mHistoryWidget, &HistoryWidget::changesCommitted, this, &GitQlientRepo::onChangesCommitted);
   connect(mHistoryWidget, &HistoryWidget::signalShowFileHistory, this, &GitQlientRepo::showFileHistory);
   connect(mHistoryWidget, &HistoryWidget::signalMergeConflicts, mControls, &Controls::activateMergeWarning);
   connect(mHistoryWidget, &HistoryWidget::signalMergeConflicts, this, &GitQlientRepo::showWarningMerge);
   connect(mHistoryWidget, &HistoryWidget::signalCherryPickConflict, mControls, &Controls::activateMergeWarning);
   connect(mHistoryWidget, &HistoryWidget::signalCherryPickConflict, this, &GitQlientRepo::showCherryPickConflict);
   connect(mHistoryWidget, &HistoryWidget::signalPullConflict, mControls, &Controls::activateMergeWarning);
   connect(mHistoryWidget, &HistoryWidget::signalPullConflict, this, &GitQlientRepo::showWarningMerge);
   connect(mHistoryWidget, &HistoryWidget::signalUpdateWip, this, &GitQlientRepo::updateWip);
   connect(mHistoryWidget, &HistoryWidget::showPrDetailedView, this, &GitQlientRepo::showGitServerPrView);

   connect(mDiffWidget, &DiffWidget::signalShowFileHistory, this, &GitQlientRepo::showFileHistory);
   connect(mDiffWidget, &DiffWidget::signalDiffEmpty, mControls, &Controls::disableDiff);
   connect(mDiffWidget, &DiffWidget::signalDiffEmpty, this, &GitQlientRepo::showPreviousView);

   connect(mBlameWidget, &BlameWidget::showFileDiff, this, &GitQlientRepo::loadFileDiff);
   connect(mBlameWidget, &BlameWidget::signalOpenDiff, this, &GitQlientRepo::openCommitCompareDiff);

   connect(mMergeWidget, &MergeWidget::signalMergeFinished, this, &GitQlientRepo::showHistoryView);
   connect(mMergeWidget, &MergeWidget::signalMergeFinished, mGitLoader.data(), &GitRepoLoader::loadAll);
   connect(mMergeWidget, &MergeWidget::signalMergeFinished, mControls, &Controls::disableMergeWarning);

   connect(mConfigWidget, &ConfigWidget::commitTitleMaxLenghtChanged, mHistoryWidget,
           &HistoryWidget::onCommitTitleMaxLenghtChanged);
   connect(mConfigWidget, &ConfigWidget::panelsVisibilityChaned, mHistoryWidget,
           &HistoryWidget::onPanelsVisibilityChanged);

   connect(mGitServerWidget, &GitServerWidget::openDiff, this, &GitQlientRepo::openCommitDiff);

   connect(mJenkins, &JenkinsWidget::gotoBranch, this, &GitQlientRepo::focusHistoryOnBranch);
   connect(mJenkins, &JenkinsWidget::gotoPullRequest, this, &GitQlientRepo::focusHistoryOnPr);

   connect(mGitLoader.data(), &GitRepoLoader::signalLoadingStarted, this, &GitQlientRepo::createProgressDialog);
   connect(mGitLoader.data(), &GitRepoLoader::signalLoadingFinished, this, &GitQlientRepo::onRepoLoadFinished);

   m_loaderThread = new QThread();
   mGitLoader->moveToThread(m_loaderThread);
   mGitQlientCache->moveToThread(m_loaderThread);
   connect(this, &GitQlientRepo::fullReload, mGitLoader.data(), &GitRepoLoader::loadAll);
   connect(this, &GitQlientRepo::referencesReload, mGitLoader.data(), &GitRepoLoader::loadReferences);
   connect(this, &GitQlientRepo::logReload, mGitLoader.data(), &GitRepoLoader::loadLogHistory);
   m_loaderThread->start();

   mGitLoader->setShowAll(mSettings->localValue("ShowAllBranches", true).toBool());
}

GitQlientRepo::~GitQlientRepo()
{
   delete mAutoFetch;
   delete mAutoFilesUpdate;

   m_loaderThread->exit();
   m_loaderThread->wait();
   delete m_loaderThread;
}

QString GitQlientRepo::currentBranch() const
{
   return mGitBase->getCurrentBranch();
}

void GitQlientRepo::updateUiFromWatcher()
{
   QLog_Info("UI", QString("Updating the GitQlient UI from watcher"));

   QScopedPointer<GitWip> git(new GitWip(mGitBase, mGitQlientCache));
   git->updateWip();

   mHistoryWidget->updateUiFromWatcher();

   mDiffWidget->reload();
}

void GitQlientRepo::setRepository(const QString &newDir)
{
   if (!newDir.isEmpty())
   {
      QLog_Info("UI", QString("Loading repository at {%1}...").arg(newDir));

      mGitLoader->cancelAll();

      emit fullReload();

      mCurrentDir = newDir;
      clearWindow();
      setWidgetsEnabled(false);
   }
   else
   {
      QLog_Info("UI", QString("Repository is empty. Cleaning GitQlient"));

      mCurrentDir = "";
      clearWindow();
      setWidgetsEnabled(false);
   }
}

void GitQlientRepo::clearWindow()
{
   blockSignals(true);

   mHistoryWidget->clear();
   mDiffWidget->clear();

   blockSignals(false);
}

void GitQlientRepo::setWidgetsEnabled(bool enabled)
{
   mControls->enableButtons(enabled);
   mHistoryWidget->setEnabled(enabled);
   mDiffWidget->setEnabled(enabled);
}

void GitQlientRepo::showFileHistory(const QString &fileName)
{
   mBlameWidget->showFileHistory(fileName);

   showBlameView();
}

void GitQlientRepo::createProgressDialog()
{
   if (!mWaitDlg)
   {
      mWaitDlg = new WaitingDlg(tr("Loading repository..."));
      mWaitDlg->setWindowFlag(Qt::Tool);
      mWaitDlg->open();

      QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
   }
}

void GitQlientRepo::onRepoLoadFinished(bool fullReload)
{
   mGitTags->getRemoteTags();

   if (!mIsInit)
   {
      mIsInit = true;

      mCurrentDir = mGitBase->getWorkingDir();

      emit repoOpened(mCurrentDir);

      setWidgetsEnabled(true);

      mBlameWidget->init(mCurrentDir);

      mControls->enableButtons(true);

      mAutoFilesUpdate->start();

      QScopedPointer<GitConfig> git(new GitConfig(mGitBase));

      if (!git->getGlobalUserInfo().isValid() && !git->getLocalUserInfo().isValid())
      {
         QLog_Info("UI", QString("Configuring Git..."));

         GitConfigDlg configDlg(mGitBase);

         configDlg.exec();

         QLog_Info("UI", QString("... Git configured!"));
      }

      QLog_Info("UI", "... repository loaded successfully");
   }

   const auto totalCommits = mGitQlientCache->commitCount();

   mHistoryWidget->loadBranches(fullReload);
   mHistoryWidget->updateGraphView(totalCommits);

   mBlameWidget->onNewRevisions(totalCommits);

   mDiffWidget->reload();

   if (mWaitDlg)
      mWaitDlg->close();

   if (QScopedPointer<GitMerge> gitMerge(new GitMerge(mGitBase, mGitQlientCache)); gitMerge->isInMerge())
   {
      mControls->activateMergeWarning();
      showWarningMerge();

      QMessageBox::warning(this, tr("Merge in progress"),
                           tr("There is a merge conflict in progress. Solve the merge before moving on."));
   }
   else if (QScopedPointer<GitLocal> gitMerge(new GitLocal(mGitBase)); gitMerge->isInCherryPickMerge())
   {
      mControls->activateMergeWarning();
      showCherryPickConflict();

      QMessageBox::warning(
          this, tr("Cherry-pick in progress"),
          tr("There is a cherry-pick in progress that contains with conflicts. Solve them before moving on."));
   }

   emit currentBranchChanged();
}

void GitQlientRepo::loadFileDiff(const QString &currentSha, const QString &previousSha, const QString &file,
                                 bool isCached)
{
   const auto loaded = mDiffWidget->loadFileDiff(currentSha, previousSha, file, isCached);

   if (loaded)
   {
      mControls->enableDiff();
      showDiffView();
   }
}

void GitQlientRepo::showHistoryView()
{
   mPreviousView = qMakePair(mControls->getCurrentSelectedButton(), mStackedLayout->currentWidget());

   mStackedLayout->setCurrentWidget(mHistoryWidget);
   mControls->toggleButton(ControlsMainViews::History);
}

void GitQlientRepo::showBlameView()
{
   mPreviousView = qMakePair(mControls->getCurrentSelectedButton(), mStackedLayout->currentWidget());

   mStackedLayout->setCurrentWidget(mBlameWidget);
   mControls->toggleButton(ControlsMainViews::Blame);
}

void GitQlientRepo::showDiffView()
{
   mPreviousView = qMakePair(mControls->getCurrentSelectedButton(), mStackedLayout->currentWidget());

   mStackedLayout->setCurrentWidget(mDiffWidget);
   mControls->toggleButton(ControlsMainViews::Diff);
}

// TODO: Optimize
void GitQlientRepo::showWarningMerge()
{
   showMergeView();

   const auto wipCommit = mGitQlientCache->commitInfo(CommitInfo::ZERO_SHA);

   QScopedPointer<GitWip> git(new GitWip(mGitBase, mGitQlientCache));
   git->updateWip();

   const auto file = mGitQlientCache->revisionFile(CommitInfo::ZERO_SHA, wipCommit.firstParent());

   if (file)
      mMergeWidget->configure(file.value(), MergeWidget::ConflictReason::Merge);
}

// TODO: Optimize
void GitQlientRepo::showCherryPickConflict(const QStringList &shas)
{
   showMergeView();

   const auto wipCommit = mGitQlientCache->commitInfo(CommitInfo::ZERO_SHA);

   QScopedPointer<GitWip> git(new GitWip(mGitBase, mGitQlientCache));
   git->updateWip();

   const auto files = mGitQlientCache->revisionFile(CommitInfo::ZERO_SHA, wipCommit.firstParent());

   if (files)
      mMergeWidget->configureForCherryPick(files.value(), shas);
}

// TODO: Optimize
void GitQlientRepo::showPullConflict()
{
   showMergeView();

   const auto wipCommit = mGitQlientCache->commitInfo(CommitInfo::ZERO_SHA);

   QScopedPointer<GitWip> git(new GitWip(mGitBase, mGitQlientCache));
   git->updateWip();

   const auto files = mGitQlientCache->revisionFile(CommitInfo::ZERO_SHA, wipCommit.firstParent());

   if (files)
      mMergeWidget->configure(files.value(), MergeWidget::ConflictReason::Pull);
}

void GitQlientRepo::showMergeView()
{
   mStackedLayout->setCurrentWidget(mMergeWidget);
   mControls->toggleButton(ControlsMainViews::Merge);
}

bool GitQlientRepo::configureGitServer() const
{
   bool isConfigured = false;

   if (!mGitServerWidget->isConfigured())
   {
      QScopedPointer<GitConfig> gitConfig(new GitConfig(mGitBase));
      const auto serverUrl = gitConfig->getServerHost();
      const auto repoInfo = gitConfig->getCurrentRepoAndOwner();

      GitQlientSettings settings("");
      const auto user = settings.globalValue(QString("%1/user").arg(serverUrl)).toString();
      const auto token = settings.globalValue(QString("%1/token").arg(serverUrl)).toString();

      GitServer::ConfigData data;
      data.user = user;
      data.token = token;
      data.serverUrl = serverUrl;
      data.repoInfo = repoInfo;

      isConfigured = mGitServerWidget->configure(data);
   }
   else
      isConfigured = true;

   return isConfigured;
}

void GitQlientRepo::showGitServerView()
{
   if (configureGitServer())
   {
      mStackedLayout->setCurrentWidget(mGitServerWidget);
      mControls->toggleButton(ControlsMainViews::GitServer);
   }
   else
      showPreviousView();
}

void GitQlientRepo::showGitServerPrView(int prNumber)
{
   if (configureGitServer())
   {
      showGitServerView();
      mGitServerWidget->openPullRequest(prNumber);
   }
}

void GitQlientRepo::showBuildSystemView()
{
   mJenkins->reload();
   mStackedLayout->setCurrentWidget(mJenkins);
   mControls->toggleButton(ControlsMainViews::BuildSystem);
}

void GitQlientRepo::showConfig()
{
   mStackedLayout->setCurrentWidget(mConfigWidget);
   mControls->toggleButton(ControlsMainViews::Config);
}

void GitQlientRepo::showPreviousView()
{
   mStackedLayout->setCurrentWidget(mPreviousView.second);
   mControls->toggleButton(mPreviousView.first);
}

void GitQlientRepo::updateWip()
{
   mHistoryWidget->resetWip();

   QScopedPointer<GitWip> git(new GitWip(mGitBase, mGitQlientCache));
   git->updateWip();

   mHistoryWidget->updateUiFromWatcher();
}

void GitQlientRepo::focusHistoryOnBranch(const QString &branch)
{
   auto found = false;
   const auto fullBranch = QString("origin/%1").arg(branch);
   auto remoteBranches = mGitQlientCache->getBranches(References::Type::RemoteBranches);

   for (const auto &remote : remoteBranches)
   {
      if (remote.second.contains(fullBranch))
      {
         found = true;
         mHistoryWidget->focusOnCommit(remote.first);
         showHistoryView();
      }
   }

   remoteBranches.clear();
   remoteBranches.squeeze();

   if (!found)
      QMessageBox::information(
          this, tr("Branch not found"),
          tr("The branch couldn't be found. Please, make sure you fetched and have the latest changes."));
}

void GitQlientRepo::focusHistoryOnPr(int prNumber)
{
   const auto pr = mGitServerCache->getPullRequest(prNumber);

   mHistoryWidget->focusOnCommit(pr.state.sha);
   showHistoryView();
}

void GitQlientRepo::openCommitDiff(const QString currentSha)
{
   const auto rev = mGitQlientCache->commitInfo(currentSha);
   const auto loaded = mDiffWidget->loadCommitDiff(currentSha, rev.firstParent());

   if (loaded)
   {
      mControls->enableDiff();

      showDiffView();
   }
}

void GitQlientRepo::openCommitCompareDiff(const QStringList &shas)
{
   const auto loaded = mDiffWidget->loadCommitDiff(shas.last(), shas.first());

   if (loaded)
   {
      mControls->enableDiff();

      showDiffView();
   }
}

void GitQlientRepo::onChangesCommitted()
{
   mHistoryWidget->selectCommit(CommitInfo::ZERO_SHA);
   mHistoryWidget->loadBranches(false);
   showHistoryView();
}

void GitQlientRepo::closeEvent(QCloseEvent *ce)
{
   QLog_Info("UI", QString("Closing GitQlient for repository {%1}").arg(mCurrentDir));

   mGitLoader->cancelAll();

   QWidget::closeEvent(ce);
}
