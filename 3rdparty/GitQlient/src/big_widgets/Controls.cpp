#include "Controls.h"

#include <BranchDlg.h>
#include <GitBase.h>
#include <GitCache.h>
#include <GitConfig.h>
#include <GitQlientSettings.h>
#include <GitQlientStyles.h>
#include <GitQlientUpdater.h>
#include <GitRemote.h>
#include <GitStashes.h>
#include <GitTags.h>
#include <PomodoroButton.h>
#include <QLogger.h>

#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>

using namespace QLogger;

Controls::Controls(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git, QWidget *parent)
   : QFrame(parent)
   , mCache(cache)
   , mGit(git)
   , mGitTags(new GitTags(mGit, mCache))
   , mHistory(new QToolButton())
   , mDiff(new QToolButton())
   , mBlame(new QToolButton())
   , mPullBtn(new QToolButton())
   , mPullOptions(new QToolButton())
   , mPushBtn(new QToolButton())
   , mRefreshBtn(new QToolButton())
   , mConfigBtn(new QToolButton())
   , mGitPlatform(new QToolButton())
   , mBuildSystem(new QToolButton())
   , mPomodoro(new PomodoroButton(mGit))
   , mVersionCheck(new QToolButton())
   , mMergeWarning(new QPushButton(tr("WARNING: There is a merge pending to be committed! Click here to solve it.")))
   , mUpdater(new GitQlientUpdater(this))
   , mBtnGroup(new QButtonGroup())
{
   setAttribute(Qt::WA_DeleteOnClose);

   connect(mUpdater, &GitQlientUpdater::newVersionAvailable, this, [this]() { mVersionCheck->setVisible(true); });

   mHistory->setCheckable(true);
   mHistory->setIcon(QIcon(":/icons/git_orange"));
   mHistory->setIconSize(QSize(22, 22));
   mHistory->setToolTip(tr("View"));
   mHistory->setToolButtonStyle(Qt::ToolButtonIconOnly);
   mBtnGroup->addButton(mHistory, static_cast<int>(ControlsMainViews::History));

   mDiff->setCheckable(true);
   mDiff->setIcon(QIcon(":/icons/diff"));
   mDiff->setIconSize(QSize(22, 22));
   mDiff->setToolTip(tr("Diff"));
   mDiff->setToolButtonStyle(Qt::ToolButtonIconOnly);
   mDiff->setEnabled(false);
   mBtnGroup->addButton(mDiff, static_cast<int>(ControlsMainViews::Diff));

   mBlame->setCheckable(true);
   mBlame->setIcon(QIcon(":/icons/blame"));
   mBlame->setIconSize(QSize(22, 22));
   mBlame->setToolTip(tr("Blame"));
   mBlame->setToolButtonStyle(Qt::ToolButtonIconOnly);
   mBtnGroup->addButton(mBlame, static_cast<int>(ControlsMainViews::Blame));

   const auto menu = new QMenu(mPullOptions);
   menu->installEventFilter(this);

   auto action = menu->addAction(tr("Fetch all"));
   connect(action, &QAction::triggered, this, &Controls::fetchAll);

   action = menu->addAction(tr("Prune"));
   connect(action, &QAction::triggered, this, &Controls::pruneBranches);
   menu->addSeparator();

   mPullBtn->setIconSize(QSize(22, 22));
   mPullBtn->setToolTip(tr("Pull"));
   mPullBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
   mPullBtn->setPopupMode(QToolButton::InstantPopup);
   mPullBtn->setIcon(QIcon(":/icons/git_pull"));
   mPullBtn->setObjectName("ToolButtonAboveMenu");

   mPullOptions->setMenu(menu);
   mPullOptions->setIcon(QIcon(":/icons/arrow_down"));
   mPullOptions->setIconSize(QSize(22, 22));
   mPullOptions->setToolButtonStyle(Qt::ToolButtonIconOnly);
   mPullOptions->setPopupMode(QToolButton::InstantPopup);
   mPullOptions->setToolTip("Remote actions");
   mPullOptions->setObjectName("ToolButtonWithMenu");

   const auto pullLayout = new QVBoxLayout();
   pullLayout->setContentsMargins(QMargins());
   pullLayout->setSpacing(0);
   pullLayout->addWidget(mPullBtn);
   pullLayout->addWidget(mPullOptions);

   mPushBtn->setIcon(QIcon(":/icons/git_push"));
   mPushBtn->setIconSize(QSize(22, 22));
   mPushBtn->setToolTip(tr("Push"));
   mPushBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);

   mRefreshBtn->setIcon(QIcon(":/icons/refresh"));
   mRefreshBtn->setIconSize(QSize(22, 22));
   mRefreshBtn->setToolTip(tr("Refresh"));
   mRefreshBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);

   mConfigBtn->setCheckable(true);
   mConfigBtn->setIcon(QIcon(":/icons/config"));
   mConfigBtn->setIconSize(QSize(22, 22));
   mConfigBtn->setToolTip(tr("Config"));
   mConfigBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
   mBtnGroup->addButton(mConfigBtn, static_cast<int>(ControlsMainViews::Config));

   const auto separator = new QFrame();
   separator->setObjectName("orangeSeparator");
   separator->setFixedHeight(20);

   const auto separator2 = new QFrame();
   separator2->setObjectName("orangeSeparator");
   separator2->setFixedHeight(20);

   const auto hLayout = new QHBoxLayout();
   hLayout->setContentsMargins(QMargins());
   hLayout->addStretch();
   hLayout->setSpacing(5);
   hLayout->addWidget(mHistory);
   hLayout->addWidget(mDiff);
   hLayout->addWidget(mBlame);
   hLayout->addWidget(separator);
   hLayout->addLayout(pullLayout);
   hLayout->addWidget(mPushBtn);
   hLayout->addWidget(separator2);

   createGitPlatformButton(hLayout);

   GitQlientSettings settings(mGit->getGitDir());
   mBuildSystem->setVisible(settings.localValue("BuildSystemEnabled", false).toBool());
   mBuildSystem->setCheckable(true);
   mBuildSystem->setIcon(QIcon(":/icons/build_system"));
   mBuildSystem->setIconSize(QSize(22, 22));
   mBuildSystem->setToolTip("Jenkins");
   mBuildSystem->setToolButtonStyle(Qt::ToolButtonIconOnly);
   mBuildSystem->setPopupMode(QToolButton::InstantPopup);
   mBtnGroup->addButton(mBuildSystem, static_cast<int>(ControlsMainViews::BuildSystem));

   connect(mBuildSystem, &QToolButton::clicked, this, &Controls::signalGoBuildSystem);

   hLayout->addWidget(mBuildSystem);

   configBuildSystemButton();

   const auto separator3 = new QFrame();
   separator3->setObjectName("orangeSeparator");
   separator3->setFixedHeight(20);
   hLayout->addWidget(separator3);

   mVersionCheck->setIcon(QIcon(":/icons/get_gitqlient"));
   mVersionCheck->setIconSize(QSize(22, 22));
   mVersionCheck->setText(tr("New version"));
   mVersionCheck->setObjectName("longToolButton");
   mVersionCheck->setToolButtonStyle(Qt::ToolButtonIconOnly);
   mVersionCheck->setVisible(false);

   mUpdater->checkNewGitQlientVersion();

   hLayout->addWidget(mRefreshBtn);
   hLayout->addWidget(mConfigBtn);
   hLayout->addWidget(mPomodoro);
   hLayout->addWidget(mVersionCheck);
   hLayout->addStretch();

   mMergeWarning->setObjectName("WarningButton");
   mMergeWarning->setVisible(false);
   mBtnGroup->addButton(mMergeWarning, static_cast<int>(ControlsMainViews::Merge));

   const auto vLayout = new QVBoxLayout(this);
   vLayout->setContentsMargins(0, 5, 0, 0);
   vLayout->setSpacing(10);
   vLayout->addLayout(hLayout);
   vLayout->addWidget(mMergeWarning);

   connect(mHistory, &QToolButton::clicked, this, &Controls::signalGoRepo);
   connect(mDiff, &QToolButton::clicked, this, &Controls::signalGoDiff);
   connect(mBlame, &QToolButton::clicked, this, &Controls::signalGoBlame);
   connect(mPullBtn, &QToolButton::clicked, this, &Controls::pullCurrentBranch);
   connect(mPushBtn, &QToolButton::clicked, this, &Controls::pushCurrentBranch);
   connect(mRefreshBtn, &QToolButton::clicked, this, &Controls::requestFullReload);
   connect(mMergeWarning, &QPushButton::clicked, this, &Controls::signalGoMerge);
   connect(mVersionCheck, &QToolButton::clicked, mUpdater, &GitQlientUpdater::showInfoMessage);
   connect(mConfigBtn, &QToolButton::clicked, this, &Controls::goConfig);

   enableButtons(false);
}

Controls::~Controls()
{
   delete mBtnGroup;
}

void Controls::toggleButton(ControlsMainViews view)
{
   mBtnGroup->button(static_cast<int>(view))->setChecked(true);
}

void Controls::enableButtons(bool enabled)
{
   mHistory->setEnabled(enabled);
   mBlame->setEnabled(enabled);
   mPullBtn->setEnabled(enabled);
   mPullOptions->setEnabled(enabled);
   mPushBtn->setEnabled(enabled);
   mRefreshBtn->setEnabled(enabled);
   mGitPlatform->setEnabled(enabled);
   mConfigBtn->setEnabled(enabled);

   if (enabled)
   {
      GitQlientSettings settings(mGit->getGitDir());
      const auto isConfigured = settings.localValue("BuildSystemEnabled", false).toBool();

      mBuildSystem->setEnabled(isConfigured);
   }
   else
      mBuildSystem->setEnabled(false);
}

void Controls::pullCurrentBranch()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->pull();
   QApplication::restoreOverrideCursor();

   if (ret.success)
   {
      if (ret.output.contains("merge conflict", Qt::CaseInsensitive))
         emit signalPullConflict();
      else
         emit requestFullReload();
   }
   else
   {
      if (ret.output.contains("error: could not apply", Qt::CaseInsensitive)
          && ret.output.contains("causing a conflict", Qt::CaseInsensitive))
      {
         emit signalPullConflict();
      }
      else
      {
         QMessageBox msgBox(QMessageBox::Critical, tr("Error while pulling"),
                            QString(tr("There were problems during the pull operation. Please, see the detailed "
                                       "description for more information.")),
                            QMessageBox::Ok, this);
         msgBox.setDetailedText(ret.output);
         msgBox.setStyleSheet(GitQlientStyles::getStyles());
         msgBox.exec();
      }
   }
}

void Controls::fetchAll()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->fetch();
   QApplication::restoreOverrideCursor();

   if (ret)
   {
      mGitTags->getRemoteTags();
      emit requestFullReload();
   }
}

void Controls::activateMergeWarning()
{
   mMergeWarning->setVisible(true);
}

void Controls::disableMergeWarning()
{
   mMergeWarning->setVisible(false);
}

void Controls::disableDiff()
{
   mDiff->setDisabled(true);
}

void Controls::enableDiff()
{
   mDiff->setEnabled(true);
}

ControlsMainViews Controls::getCurrentSelectedButton() const
{
   return mBlame->isChecked() ? ControlsMainViews::Blame : ControlsMainViews::History;
}

void Controls::pushCurrentBranch()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->push();
   QApplication::restoreOverrideCursor();

   if (ret.output.contains("has no upstream branch"))
   {
      const auto currentBranch = mGit->getCurrentBranch();
      BranchDlg dlg({ currentBranch, BranchDlgMode::PUSH_UPSTREAM, mCache, mGit });
      const auto dlgRet = dlg.exec();

      if (dlgRet == QDialog::Accepted)
         emit signalRefreshPRsCache();
   }
   else if (ret.success)
   {
      const auto currentBranch = mGit->getCurrentBranch();
      QScopedPointer<GitConfig> git(new GitConfig(mGit));
      const auto remote = git->getRemoteForBranch(currentBranch);

      if (remote.success)
      {
         const auto oldSha = mCache->getShaOfReference(QString("%1/%2").arg(remote.output, currentBranch),
                                                       References::Type::RemoteBranches);
         const auto sha = mCache->getShaOfReference(currentBranch, References::Type::LocalBranch);
         mCache->deleteReference(oldSha, References::Type::RemoteBranches,
                                 QString("%1/%2").arg(remote.output, currentBranch));
         mCache->insertReference(sha, References::Type::RemoteBranches,
                                 QString("%1/%2").arg(remote.output, currentBranch));
         emit mCache->signalCacheUpdated();
         emit signalRefreshPRsCache();
      }
   }
   else
   {
      QMessageBox msgBox(
          QMessageBox::Critical, tr("Error while pushing"),
          QString(tr("There were problems during the push operation. Please, see the detailed description "
                     "for more information.")),
          QMessageBox::Ok, this);
      msgBox.setDetailedText(ret.output);
      msgBox.setStyleSheet(GitQlientStyles::getStyles());
      msgBox.exec();
   }
}

void Controls::pruneBranches()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->prune();
   QApplication::restoreOverrideCursor();

   if (ret.success)
      emit requestReferencesReload();
}

void Controls::createGitPlatformButton(QHBoxLayout *layout)
{
   QScopedPointer<GitConfig> gitConfig(new GitConfig(mGit));
   const auto remoteUrl = gitConfig->getServerHost();
   QIcon gitPlatformIcon;
   QString name;
   QString prName;
   auto add = false;

   if (remoteUrl.contains("github", Qt::CaseInsensitive))
   {
      add = true;

      gitPlatformIcon = QIcon(":/icons/github");
      name = "GitHub";
      prName = tr("Pull Request");
   }
   else if (remoteUrl.contains("gitlab", Qt::CaseInsensitive))
   {
      add = true;

      gitPlatformIcon = QIcon(":/icons/gitlab");
      name = "GitLab";
      prName = tr("Merge Request");
   }

   if (add)
   {
      mGitPlatform->setCheckable(true);
      mGitPlatform->setIcon(gitPlatformIcon);
      mGitPlatform->setIconSize(QSize(22, 22));
      mGitPlatform->setToolTip(name);
      mGitPlatform->setToolButtonStyle(Qt::ToolButtonIconOnly);
      mGitPlatform->setPopupMode(QToolButton::InstantPopup);
      mBtnGroup->addButton(mGitPlatform, static_cast<int>(ControlsMainViews::GitServer));

      layout->addWidget(mGitPlatform);

      connect(mGitPlatform, &QToolButton::clicked, this, &Controls::signalGoServer);
   }
   else
      mGitPlatform->setVisible(false);
}

void Controls::configBuildSystemButton()
{
   GitQlientSettings settings(mGit->getGitDir());
   const auto isConfigured = settings.localValue("BuildSystemEnabled", false).toBool();
   mBuildSystem->setEnabled(isConfigured);

   if (!isConfigured)
      emit signalGoRepo();
}

bool Controls::eventFilter(QObject *obj, QEvent *event)
{
   if (const auto menu = qobject_cast<QMenu *>(obj); menu && event->type() == QEvent::Show)
   {
      auto localPos = menu->parentWidget()->pos();
      localPos.setX(localPos.x());
      auto pos = mapToGlobal(localPos);
      menu->show();
      pos.setY(pos.y() + menu->parentWidget()->height());
      menu->move(pos);
      return true;
   }

   return false;
}
