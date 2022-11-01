#include "BranchContextMenu.h"

#include <BranchDlg.h>
#include <GitBase.h>
#include <GitBranches.h>
#include <GitCache.h>
#include <GitConfig.h>
#include <GitQlientStyles.h>
#include <GitRemote.h>

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>

#include <utility>

BranchContextMenu::BranchContextMenu(BranchContextMenuConfig config, QWidget *parent)
   : QMenu(parent)
   , mConfig(std::move(config))
{
   setAttribute(Qt::WA_DeleteOnClose);

   connect(addAction(tr("Copy name")), &QAction::triggered, this,
           [this] { QApplication::clipboard()->setText(mConfig.branchSelected); });

   addSeparator();

   if (mConfig.isLocal)
   {
      connect(addAction(tr("Pull")), &QAction::triggered, this, &BranchContextMenu::pull);
      connect(addAction(tr("Fetch")), &QAction::triggered, this, &BranchContextMenu::fetch);
      connect(addAction(tr("Push")), &QAction::triggered, this, &BranchContextMenu::push);
   }

   if (mConfig.currentBranch == mConfig.branchSelected)
      connect(addAction(tr("Push force")), &QAction::triggered, this, &BranchContextMenu::pushForce);

   addSeparator();

   connect(addAction(tr("Create branch")), &QAction::triggered, this, &BranchContextMenu::createBranch);
   connect(addAction(tr("Create && checkout branch")), &QAction::triggered, this,
           &BranchContextMenu::createCheckoutBranch);
   connect(addAction(tr("Checkout branch")), &QAction::triggered, this, &BranchContextMenu::signalCheckoutBranch);

   if (mConfig.currentBranch != mConfig.branchSelected)
   {
      const auto actionName = tr("Merge %1 into %2").arg(mConfig.branchSelected, mConfig.currentBranch);
      connect(addAction(actionName), &QAction::triggered, this, &BranchContextMenu::merge);

      const auto mergeSquashAction = tr("Squash-merge %1 into %2").arg(mConfig.branchSelected, mConfig.currentBranch);
      connect(addAction(mergeSquashAction), &QAction::triggered, this, &BranchContextMenu::mergeSquash);
   }

   addSeparator();

   connect(addAction(tr("Rename")), &QAction::triggered, this, &BranchContextMenu::rename);
   connect(addAction(tr("Delete")), &QAction::triggered, this, &BranchContextMenu::deleteBranch);
}

void BranchContextMenu::pull()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mConfig.mGit));
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

void BranchContextMenu::fetch()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mConfig.mGit));
   const auto ret = git->fetch();
   QApplication::restoreOverrideCursor();

   if (ret)
   {
      emit signalFetchPerformed();
      emit fullReload();
   }
   else
      QMessageBox::critical(this, tr("Fetch failed"), tr("There were some problems while fetching. Please try again."));
}

void BranchContextMenu::push()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mConfig.mGit));
   const auto ret
       = mConfig.currentBranch == mConfig.branchSelected ? git->push() : git->pushBranch(mConfig.branchSelected);
   QApplication::restoreOverrideCursor();

   if (ret.output.contains("has no upstream branch"))
   {
      BranchDlg dlg({ mConfig.branchSelected, BranchDlgMode::PUSH_UPSTREAM, mConfig.mCache, mConfig.mGit });
      dlg.exec();
   }
   else if (ret.success)
   {
      QScopedPointer<GitConfig> git(new GitConfig(mConfig.mGit));
      const auto remote = git->getRemoteForBranch(mConfig.branchSelected);

      if (remote.success)
      {
         const auto oldSha = mConfig.mCache->getShaOfReference(
             QString("%1/%2").arg(remote.output, mConfig.branchSelected), References::Type::RemoteBranches);
         const auto sha = mConfig.mCache->getShaOfReference(mConfig.branchSelected, References::Type::LocalBranch);
         mConfig.mCache->deleteReference(oldSha, References::Type::RemoteBranches,
                                         QString("%1/%2").arg(remote.output, mConfig.branchSelected));
         mConfig.mCache->insertReference(sha, References::Type::RemoteBranches,
                                         QString("%1/%2").arg(remote.output, mConfig.branchSelected));
         emit mConfig.mCache->signalCacheUpdated();
         emit logReload();
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

void BranchContextMenu::pushForce()
{
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   QScopedPointer<GitRemote> git(new GitRemote(mConfig.mGit));
   const auto ret = git->push(true);
   QApplication::restoreOverrideCursor();

   if (ret.success)
   {
      emit signalRefreshPRsCache();
      emit fullReload();
   }
   else
   {
      QMessageBox msgBox(QMessageBox::Critical, tr("Error while pulling"),
                         tr("There were problems during the pull operation. Please, see the detailed description "
                            "for more information."),
                         QMessageBox::Ok, this);
      msgBox.setDetailedText(ret.output);
      msgBox.setStyleSheet(GitQlientStyles::getStyles());
      msgBox.exec();
   }
}

void BranchContextMenu::createBranch()
{
   BranchDlg dlg({ mConfig.branchSelected, BranchDlgMode::CREATE, mConfig.mCache, mConfig.mGit });
   dlg.exec();
}

void BranchContextMenu::createCheckoutBranch()
{
   BranchDlg dlg({ mConfig.branchSelected, BranchDlgMode::CREATE_CHECKOUT, mConfig.mCache, mConfig.mGit });
   dlg.exec();
}

void BranchContextMenu::merge()
{
   emit signalMergeRequired(mConfig.currentBranch, mConfig.branchSelected);
}

void BranchContextMenu::mergeSquash()
{
   emit mergeSqushRequested(mConfig.currentBranch, mConfig.branchSelected);
}

void BranchContextMenu::rename()
{
   BranchDlg dlg({ mConfig.branchSelected, BranchDlgMode::RENAME, mConfig.mCache, mConfig.mGit });
   dlg.exec();
}

void BranchContextMenu::deleteBranch()
{
   if (!mConfig.isLocal && mConfig.branchSelected == "master")
      QMessageBox::critical(this, tr("Delete master?!"), tr("You are not allowed to delete remote master."),
                            QMessageBox::Ok);
   else
   {
      auto ret = QMessageBox::warning(this, tr("Delete branch!"), tr("Are you sure you want to delete the branch?"),
                                      QMessageBox::Ok, QMessageBox::Cancel);

      if (ret == QMessageBox::Ok)
      {
         const auto type = mConfig.isLocal ? References::Type::LocalBranch : References::Type::RemoteBranches;
         const auto sha = mConfig.mCache->getShaOfReference(mConfig.branchSelected, type);
         QScopedPointer<GitBranches> git(new GitBranches(mConfig.mGit));
         QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
         const auto ret2 = mConfig.isLocal ? git->removeLocalBranch(mConfig.branchSelected)
                                           : git->removeRemoteBranch(mConfig.branchSelected);
         QApplication::restoreOverrideCursor();

         if (ret2.success)
         {
            mConfig.mCache->deleteReference(sha, type, mConfig.branchSelected);
            emit mConfig.mCache->signalCacheUpdated();
            emit logReload();
         }
         else
            QMessageBox::critical(
                this, tr("Delete a branch failed"),
                tr("There were some problems while deleting the branch:<br><br> %1").arg(ret2.output));
      }
   }
}
