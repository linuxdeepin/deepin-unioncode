#include "CommitHistoryContextMenu.h"

#include <BranchDlg.h>
#include <CommitInfo.h>
//#include <CreateIssueDlg.h>
//#include <CreatePullRequestDlg.h>
#include <GitBase.h>
#include <GitBranches.h>
#include <GitCache.h>
#include <GitConfig.h>
#include <GitHistory.h>
#include <GitHubRestApi.h>
#include <GitLocal.h>
#include <GitPatches.h>
#include <GitQlientStyles.h>
#include <GitRemote.h>
#include <GitServerCache.h>
#include <GitStashes.h>
#include <GitTags.h>
#include <MergePullRequestDlg.h>
#include <PullDlg.h>
#include <SquashDlg.h>
#include <TagDlg.h>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

#include <QLogger.h>

using namespace QLogger;

CommitHistoryContextMenu::CommitHistoryContextMenu(const QSharedPointer<GitCache> &cache,
                                                   const QSharedPointer<GitBase> &git,
                                                   const QSharedPointer<GitServerCache> &gitServerCache,
                                                   const QStringList &shas, QWidget *parent)
   : QMenu(parent)
   , mCache(cache)
   , mGit(git)
   , mGitServerCache(gitServerCache)
   , mGitTags(new GitTags(mGit, mCache))
   , mShas(shas)
{
   setAttribute(Qt::WA_DeleteOnClose);

   if (shas.count() == 1)
      createIndividualShaMenu();
   else
      createMultipleShasMenu();
}

void CommitHistoryContextMenu::createIndividualShaMenu()
{
   const auto singleSelection = mShas.count() == 1;

   if (singleSelection)
   {
      const auto sha = mShas.first();

      if (sha == CommitInfo::ZERO_SHA)
      {
         const auto stashMenu = addMenu(tr("Stash"));
         const auto stashAction = stashMenu->addAction(tr("Push"));
         connect(stashAction, &QAction::triggered, this, &CommitHistoryContextMenu::stashPush);

         const auto popAction = stashMenu->addAction(tr("Pop"));
         connect(popAction, &QAction::triggered, this, &CommitHistoryContextMenu::stashPop);
      }

      const auto commitAction = addAction(tr("See diff"));
      connect(commitAction, &QAction::triggered, this, [this]() { emit signalOpenDiff(mShas.first()); });

      if (sha != CommitInfo::ZERO_SHA)
      {
         const auto createMenu = addMenu(tr("Create"));

         const auto createBranchAction = createMenu->addAction(tr("Branch"));
         connect(createBranchAction, &QAction::triggered, this, &CommitHistoryContextMenu::createBranch);

         const auto createTagAction = createMenu->addAction(tr("Tag"));
         connect(createTagAction, &QAction::triggered, this, &CommitHistoryContextMenu::createTag);

         const auto exportAsPatchAction = addAction(tr("Export as patch"));
         connect(exportAsPatchAction, &QAction::triggered, this, &CommitHistoryContextMenu::exportAsPatch);

         addSeparator();

         const auto checkoutCommitAction = addAction(tr("Checkout commit"));
         connect(checkoutCommitAction, &QAction::triggered, this, &CommitHistoryContextMenu::checkoutCommit);

         addBranchActions(sha);

         QScopedPointer<GitBranches> git(new GitBranches(mGit));

         if (auto ret = git->getLastCommitOfBranch(mGit->getCurrentBranch()); ret.success)
         {
            const auto lastShaStr = ret.output.trimmed();

            if (lastShaStr == sha)
            {
               const auto amendCommitAction = addAction(tr("Amend"));
               connect(amendCommitAction, &QAction::triggered, this,
                       [this]() { emit signalAmendCommit(mShas.first()); });

               const auto applyMenu = addMenu(tr("Apply"));

               const auto applyPatchAction = applyMenu->addAction(tr("Patch"));
               connect(applyPatchAction, &QAction::triggered, this, &CommitHistoryContextMenu::applyPatch);

               const auto applyCommitAction = applyMenu->addAction(tr("Commit"));
               connect(applyCommitAction, &QAction::triggered, this, &CommitHistoryContextMenu::applyCommit);

               const auto pushAction = addAction(tr("Push"));
               connect(pushAction, &QAction::triggered, this, &CommitHistoryContextMenu::push);

               const auto pullAction = addAction(tr("Pull"));
               connect(pullAction, &QAction::triggered, this, &CommitHistoryContextMenu::pull);

               const auto fetchAction = addAction(tr("Fetch"));
               connect(fetchAction, &QAction::triggered, this, &CommitHistoryContextMenu::fetch);
            }
            else if (mCache->isCommitInCurrentGeneologyTree(mShas.first()))
            {
               const auto pushAction = addAction(tr("Push"));
               connect(pushAction, &QAction::triggered, this, &CommitHistoryContextMenu::push);
            }
         }

         const auto resetMenu = addMenu(tr("Reset"));

         const auto resetSoftAction = resetMenu->addAction(tr("Soft"));
         connect(resetSoftAction, &QAction::triggered, this, &CommitHistoryContextMenu::resetSoft);

         const auto resetMixedAction = resetMenu->addAction(tr("Mixed"));
         connect(resetMixedAction, &QAction::triggered, this, &CommitHistoryContextMenu::resetMixed);

         const auto resetHardAction = resetMenu->addAction(tr("Hard"));
         connect(resetHardAction, &QAction::triggered, this, &CommitHistoryContextMenu::resetHard);

         addSeparator();

         const auto copyMenu = addMenu(tr("Copy"));

         const auto copyShaAction = copyMenu->addAction(tr("Commit SHA"));
         connect(copyShaAction, &QAction::triggered, this,
                 [this]() { QApplication::clipboard()->setText(mShas.first()); });

         const auto copyTitleAction = copyMenu->addAction(tr("Commit title"));
         connect(copyTitleAction, &QAction::triggered, this, [this]() {
            const auto title = mCache->commitInfo(mShas.first()).shortLog;
            QApplication::clipboard()->setText(title);
         });
      }
   }

   if (mGitServerCache)
   {
      const auto isGitHub = mGitServerCache->getPlatform() == GitServer::Platform::GitHub;
      const auto gitServerMenu = new QMenu(QString::fromUtf8(isGitHub ? "GitHub" : "GitLab"), this);

      addSeparator();
      addMenu(gitServerMenu);

      if (const auto pr = mGitServerCache->getPullRequest(mShas.first()); singleSelection && pr.isValid())
      {
         const auto prInfo = mGitServerCache->getPullRequest(mShas.first());

         const auto checksMenu = new QMenu("Checks", gitServerMenu);
         gitServerMenu->addMenu(checksMenu);

         for (const auto &check : prInfo.state.checks)
         {
            const auto link = check.url;
            checksMenu->addAction(QIcon(QString(":/icons/%1").arg(check.state)), check.name, this,
                                  [link]() { QDesktopServices::openUrl(link); });
         }

         if (isGitHub)
         {
            connect(gitServerMenu->addAction(tr("Merge PR")), &QAction::triggered, this, [this, pr]() {
               const auto mergeDlg = new MergePullRequestDlg(mGit, pr, mShas.first(), this);
               connect(mergeDlg, &MergePullRequestDlg::signalRepositoryUpdated, this,
                       &CommitHistoryContextMenu::fullReload);

               mergeDlg->exec();
            });
         }

         connect(gitServerMenu->addAction(tr("Show PR detailed view")), &QAction::triggered, this,
                 [this, num = pr.number]() { emit showPrDetailedView(num); });

         gitServerMenu->addSeparator();
      }
   }
}

void CommitHistoryContextMenu::createMultipleShasMenu()
{
   if (mShas.count() == 2)
   {
      const auto diffAction = addAction(tr("See diff"));
      connect(diffAction, &QAction::triggered, this, [this]() { emit signalOpenCompareDiff(mShas); });
   }

   if (!mShas.contains(CommitInfo::ZERO_SHA))
   {
      const auto exportAsPatchAction = addAction(tr("Export as patch"));
      connect(exportAsPatchAction, &QAction::triggered, this, &CommitHistoryContextMenu::exportAsPatch);

      const auto copyShaAction = addAction(tr("Copy all SHA"));
      connect(copyShaAction, &QAction::triggered, this,
              [this]() { QApplication::clipboard()->setText(mShas.join(',')); });

      auto shasInCurrenTree = 0;

      for (const auto &sha : qAsConst(mShas))
         shasInCurrenTree += mCache->isCommitInCurrentGeneologyTree(sha);

      if (shasInCurrenTree == 0)
      {
         const auto cherryPickAction = addAction(tr("Cherry pick ALL commits"));
         connect(cherryPickAction, &QAction::triggered, this, &CommitHistoryContextMenu::cherryPickCommit);
      }
      else if (shasInCurrenTree == mShas.count())
      {
         const auto cherryPickAction = addAction(tr("Squash commits"));
         connect(cherryPickAction, &QAction::triggered, this, &CommitHistoryContextMenu::showSquashDialog);
      }
   }
   else
      QLog_Warning("UI", "WIP selected as part of a series of SHAs");
}

void CommitHistoryContextMenu::stashPush()
{
   QScopedPointer<GitStashes> git(new GitStashes(mGit));
   const auto ret = git->stash();

   if (ret.success)
      emit logReload();
}

void CommitHistoryContextMenu::stashPop()
{
   QScopedPointer<GitStashes> git(new GitStashes(mGit));
   const auto ret = git->pop();

   if (ret.success)
      emit logReload();
}

void CommitHistoryContextMenu::createBranch()
{
   BranchDlg dlg({ mShas.first(), BranchDlgMode::CREATE_FROM_COMMIT, mCache, mGit });
   dlg.exec();
}

void CommitHistoryContextMenu::createTag()
{
   TagDlg dlg(QSharedPointer<GitBase>::create(mGit->getWorkingDir()), mShas.first());
   const auto ret = dlg.exec();

   if (ret == QDialog::Accepted)
      emit referencesReload(); // TODO: Optimize
}

void CommitHistoryContextMenu::exportAsPatch()
{
   QScopedPointer<GitPatches> git(new GitPatches(mGit));
   const auto ret = git->exportPatch(mShas);

   if (ret.success)
   {
      const auto action = QMessageBox::information(this, tr("Patch generated"),
                                                   tr("<p>The patch has been generated!</p>"
                                                      "<p><b>Commit:</b></p><p>%1</p>"
                                                      "<p><b>Destination:</b> %2</p>"
                                                      "<p><b>File names:</b></p><p>%3</p>")
                                                       .arg(mShas.join("<br>"), mGit->getWorkingDir(), ret.output),
                                                   QMessageBox::Ok, QMessageBox::Open);

      if (action == QMessageBox::Open)
      {
         QString fileBrowser;

#ifdef Q_OS_LINUX
         fileBrowser.append("xdg-open");
#elif defined(Q_OS_WIN)
         fileBrowser.append("explorer.exe");
#endif

         QProcess::startDetached(fileBrowser, { mGit->getWorkingDir() });
      }
   }
}

void CommitHistoryContextMenu::checkoutBranch()
{
   const auto action = qobject_cast<QAction *>(sender());
   auto isLocal = action->data().toBool();
   auto branchName = action->text();

   if (isLocal)
      branchName.remove("origin/");

   QScopedPointer<GitBranches> git(new GitBranches(mGit));
   const auto ret = isLocal ? git->checkoutLocalBranch(branchName) : git->checkoutRemoteBranch(branchName);
   const auto output = ret.output;

   if (ret.success)
   {
      QRegExp rx("by \\d+ commits");
      rx.indexIn(ret.output);
      auto value = rx.capturedTexts().constFirst().split(" ");

      if (value.count() == 3 && output.contains("your branch is behind", Qt::CaseInsensitive))
      {
         const auto commits = value.at(1).toUInt();
         (void)commits;

         PullDlg pull(mGit, output.split('\n').first());
         connect(&pull, &PullDlg::signalRepositoryUpdated, this, &CommitHistoryContextMenu::fullReload);
         connect(&pull, &PullDlg::signalPullConflict, this, &CommitHistoryContextMenu::signalPullConflict);
      }

      emit logReload();
   }
   else
   {
      QMessageBox msgBox(QMessageBox::Critical, tr("Error while checking out"),
                         tr("There were problems during the checkout operation. Please, see the detailed "
                            "description for more information."),
                         QMessageBox::Ok, this);
      msgBox.setDetailedText(ret.output);
      msgBox.setStyleSheet(GitQlientStyles::getStyles());
      msgBox.exec();
   }
}

void CommitHistoryContextMenu::createCheckoutBranch()
{
   BranchDlg dlg({ mShas.constFirst(), BranchDlgMode::CREATE_CHECKOUT_FROM_COMMIT, mCache, mGit });
   dlg.exec();
}

void CommitHistoryContextMenu::checkoutCommit()
{
   const auto sha = mShas.first();
   QLog_Info("UI", QString("Checking out the commit {%1}").arg(sha));

   QScopedPointer<GitLocal> git(new GitLocal(mGit));
   const auto ret = git->checkoutCommit(sha);

   if (ret.success)
      emit logReload();
   else
   {
      QMessageBox msgBox(QMessageBox::Critical, tr("Error while checking out"),
                         tr("There were problems during the checkout operation. Please, see the detailed "
                            "description for more information."),
                         QMessageBox::Ok, this);
      msgBox.setDetailedText(ret.output);
      msgBox.setStyleSheet(GitQlientStyles::getStyles());
      msgBox.exec();
   }
}

void CommitHistoryContextMenu::cherryPickCommit()
{
   auto shas = mShas;
   for (const auto &sha : qAsConst(mShas))
   {
      const auto lastShaBeforeCommit = mGit->getLastCommit().output.trimmed();
      QScopedPointer<GitLocal> git(new GitLocal(mGit));
      const auto ret = git->cherryPickCommit(sha);

      shas.takeFirst();

      if (ret.success && shas.isEmpty())
      {
         auto commit = mCache->commitInfo(sha);
         commit.sha = mGit->getLastCommit().output.trimmed();

         mCache->insertCommit(commit);
         mCache->deleteReference(lastShaBeforeCommit, References::Type::LocalBranch, mGit->getCurrentBranch());
         mCache->insertReference(commit.sha, References::Type::LocalBranch, mGit->getCurrentBranch());

         QScopedPointer<GitHistory> gitHistory(new GitHistory(mGit));
         const auto ret = gitHistory->getDiffFiles(commit.sha, lastShaBeforeCommit);

         mCache->insertRevisionFiles(commit.sha, lastShaBeforeCommit, RevisionFiles(ret.output));

         emit mCache->signalCacheUpdated();
         emit logReload();
      }
      else if (!ret.success)
      {
         const auto errorMsg = ret.output;

         if (errorMsg.contains("error: could not apply", Qt::CaseInsensitive)
             && errorMsg.contains("after resolving the conflicts", Qt::CaseInsensitive))
         {
            emit signalCherryPickConflict(shas);
         }
         else
         {
            QMessageBox msgBox(QMessageBox::Critical, tr("Error while cherry-pick"),
                               tr("There were problems during the cherry-pich operation. Please, see the detailed "
                                  "description for more information."),
                               QMessageBox::Ok, this);
            msgBox.setDetailedText(errorMsg);
            msgBox.setStyleSheet(GitQlientStyles::getStyles());
            msgBox.exec();
         }
      }
   }
}

void CommitHistoryContextMenu::applyPatch()
{
   const QString fileName(QFileDialog::getOpenFileName(this, tr("Select a patch to apply")));
   QScopedPointer<GitPatches> git(new GitPatches(mGit));

   if (!fileName.isEmpty() && git->applyPatch(fileName))
      emit logReload();
}

void CommitHistoryContextMenu::applyCommit()
{
   const QString fileName(QFileDialog::getOpenFileName(this, "Select a patch to apply"));
   QScopedPointer<GitPatches> git(new GitPatches(mGit));

   if (!fileName.isEmpty() && git->applyPatch(fileName, true))
      emit logReload();
}

void CommitHistoryContextMenu::push()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->pushCommit(mShas.first(), mGit->getCurrentBranch());
   QApplication::restoreOverrideCursor();

   if (ret.output.contains("has no upstream branch"))
   {
      const auto currentBranch = mGit->getCurrentBranch();
      BranchDlg dlg({ currentBranch, BranchDlgMode::PUSH_UPSTREAM, mCache, mGit });
      const auto ret = dlg.exec();

      if (ret == QDialog::Accepted)
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
      QMessageBox msgBox(QMessageBox::Critical, tr("Error while pushing"),
                         tr("There were problems during the push operation. Please, see the detailed description "
                            "for more information."),
                         QMessageBox::Ok, this);
      msgBox.setDetailedText(ret.output);
      msgBox.setStyleSheet(GitQlientStyles::getStyles());
      msgBox.exec();
   }
}

void CommitHistoryContextMenu::pull()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto ret = git->pull();
   QApplication::restoreOverrideCursor();

   if (ret.success)
      emit fullReload();
   else
   {
      const auto errorMsg = ret.output;

      if (errorMsg.contains("error: could not apply", Qt::CaseInsensitive)
          && errorMsg.contains("causing a conflict", Qt::CaseInsensitive))
      {
         emit signalPullConflict();
      }
      else
      {
         QMessageBox msgBox(QMessageBox::Critical, tr("Error while pulling"),
                            tr("There were problems during the pull operation. Please, see the detailed "
                               "description for more information."),
                            QMessageBox::Ok, this);
         msgBox.setDetailedText(errorMsg);
         msgBox.setStyleSheet(GitQlientStyles::getStyles());
         msgBox.exec();
      }
   }
}

void CommitHistoryContextMenu::fetch()
{
   QScopedPointer<GitRemote> git(new GitRemote(mGit));

   if (git->fetch())
   {
      mGitTags->getRemoteTags();
      emit fullReload();
   }
}

void CommitHistoryContextMenu::resetSoft()
{
   QScopedPointer<GitLocal> git(new GitLocal(mGit));
   const auto previousSha = mGit->getLastCommit().output.trimmed();

   if (git->resetCommit(mShas.first(), GitLocal::CommitResetType::SOFT))
   {
      mCache->deleteReference(previousSha, References::Type::LocalBranch, mGit->getCurrentBranch());
      mCache->insertReference(mShas.first(), References::Type::LocalBranch, mGit->getCurrentBranch());

      emit logReload();
   }
}

void CommitHistoryContextMenu::resetMixed()
{
   QScopedPointer<GitLocal> git(new GitLocal(mGit));
   const auto previousSha = mGit->getLastCommit().output.trimmed();

   if (git->resetCommit(mShas.first(), GitLocal::CommitResetType::MIXED))
   {
      mCache->deleteReference(previousSha, References::Type::LocalBranch, mGit->getCurrentBranch());
      mCache->insertReference(mShas.first(), References::Type::LocalBranch, mGit->getCurrentBranch());

      emit logReload();
   }
}

void CommitHistoryContextMenu::resetHard()
{
   const auto retMsg = QMessageBox::warning(
       this, "Reset hard requested!", "Are you sure you want to reset the branch to this commit in a <b>hard</b> way?",
       QMessageBox::Ok, QMessageBox::Cancel);

   if (retMsg == QMessageBox::Ok)
   {
      const auto previousSha = mGit->getLastCommit().output.trimmed();
      QScopedPointer<GitLocal> git(new GitLocal(mGit));

      if (git->resetCommit(mShas.first(), GitLocal::CommitResetType::HARD))
      {
         mCache->deleteReference(previousSha, References::Type::LocalBranch, mGit->getCurrentBranch());
         mCache->insertReference(mShas.first(), References::Type::LocalBranch, mGit->getCurrentBranch());

         emit logReload();
      }
   }
}

void CommitHistoryContextMenu::merge()
{
   const auto action = qobject_cast<QAction *>(sender());
   const auto fromBranch = action->data().toString();

   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto currentBranch = mGit->getCurrentBranch();

   emit signalMergeRequired(currentBranch, fromBranch);
}

void CommitHistoryContextMenu::mergeSquash()
{
   const auto action = qobject_cast<QAction *>(sender());
   const auto fromBranch = action->data().toString();

   QScopedPointer<GitRemote> git(new GitRemote(mGit));
   const auto currentBranch = mGit->getCurrentBranch();

   emit mergeSqushRequested(currentBranch, fromBranch);
}

void CommitHistoryContextMenu::addBranchActions(const QString &sha)
{
   auto remoteBranches = mCache->getReferences(sha, References::Type::RemoteBranches);
   const auto localBranches = mCache->getReferences(sha, References::Type::LocalBranch);

   QMap<QString, bool> branchTracking;

   if (remoteBranches.isEmpty())
   {
      for (const auto &branch : localBranches)
         branchTracking.insert(branch, true);
   }
   else
   {
      for (const auto &local : localBranches)
      {
         const auto iter = std::find_if(remoteBranches.begin(), remoteBranches.end(), [local](const QString &remote) {
            if (remote.contains(local))
               return true;
            return false;
         });

         branchTracking.insert(local, true);

         if (iter != remoteBranches.end())
            remoteBranches.erase(iter);
      }
      for (const auto &remote : remoteBranches)
         branchTracking.insert(remote, false);
   }

   QList<QAction *> branchesToCheckout;
   const auto currentBranch = mGit->getCurrentBranch();

   for (const auto &pair : branchTracking.toStdMap())
   {
      if (!branchTracking.isEmpty() && pair.first != currentBranch
          && pair.first != QString("origin/%1").arg(currentBranch))
      {
         const auto checkoutCommitAction = new QAction(QString(tr("%1")).arg(pair.first));
         checkoutCommitAction->setData(pair.second);
         connect(checkoutCommitAction, &QAction::triggered, this, &CommitHistoryContextMenu::checkoutBranch);
         branchesToCheckout.append(checkoutCommitAction);
      }
   }

   const auto branchMenu = !branchesToCheckout.isEmpty() ? addMenu(tr("Checkout branch")) : this;
   const auto newBranchAction
       = branchMenu->addAction(!branchesToCheckout.isEmpty() ? tr("New Branch") : tr("Checkout new branch"));
   connect(newBranchAction, &QAction::triggered, this, &CommitHistoryContextMenu::createCheckoutBranch);

   if (!branchesToCheckout.isEmpty())
   {
      branchMenu->addSeparator();
      branchMenu->addActions(branchesToCheckout);
   }

   if (!mCache->isCommitInCurrentGeneologyTree(sha))
   {
      for (const auto &pair : branchTracking.toStdMap())
      {
         if (!pair.first.isEmpty() && pair.first != currentBranch
             && pair.first != QString("origin/%1").arg(currentBranch))
         {
            // If is the last commit of a branch
            const auto mergeBranchAction = addAction(QString(tr("Merge %1")).arg(pair.first));
            mergeBranchAction->setData(pair.first);
            connect(mergeBranchAction, &QAction::triggered, this, &CommitHistoryContextMenu::merge);

            const auto mergeSquashBranchAction = addAction(QString(tr("Squash-merge %1")).arg(pair.first));
            mergeSquashBranchAction->setData(pair.first);
            connect(mergeSquashBranchAction, &QAction::triggered, this, &CommitHistoryContextMenu::mergeSquash);
         }
      }

      addSeparator();

      const auto cherryPickAction = addAction(tr("Cherry pick commit"));
      connect(cherryPickAction, &QAction::triggered, this, &CommitHistoryContextMenu::cherryPickCommit);
   }
   else
      addSeparator();
}

void CommitHistoryContextMenu::showSquashDialog()
{
   if (mCache->pendingLocalChanges())
   {
      QMessageBox::warning(this, tr("Squash not possible"),
                           tr("Please, make sure there are no pending changes to be commited."));
   }
   else
   {
      const auto squash = new SquashDlg(mGit, mCache, mShas, this);
      connect(squash, &SquashDlg::changesCommitted, this, &CommitHistoryContextMenu::fullReload);
      squash->exec();
   }
}
