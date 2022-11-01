#include <WipWidget.h>
#include <ui_CommitChangesWidget.h>

#include <FileWidget.h>
#include <GitBase.h>
#include <GitCache.h>
#include <GitConfig.h>
#include <GitHistory.h>
#include <GitLocal.h>
#include <GitQlientRole.h>
#include <GitQlientStyles.h>
#include <GitRepoLoader.h>
#include <GitWip.h>
#include <UnstagedMenu.h>

#include <QMessageBox>

#include <QLogger.h>

using namespace QLogger;

WipWidget::WipWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git, QWidget *parent)
   : CommitChangesWidget(cache, git, parent)
{
   mCurrentSha = CommitInfo::ZERO_SHA;
}

void WipWidget::configure(const QString &sha)
{
   const auto commit = mCache->commitInfo(sha);

   QScopedPointer<GitWip> git(new GitWip(mGit, mCache));
   git->updateWip();

   const auto files = mCache->revisionFile(CommitInfo::ZERO_SHA, commit.firstParent());

   QLog_Info("UI", QString("Configuring WIP widget"));

   prepareCache();

   if (files)
      insertFiles(files.value(), ui->unstagedFilesList);

   clearCache();

   ui->applyActionBtn->setEnabled(ui->stagedFilesList->count());
}

void WipWidget::commitChanges()
{
   QString msg;
   QStringList selFiles = getFiles();

   if (!selFiles.isEmpty())
   {
      if (hasConflicts())
         QMessageBox::warning(this, tr("Impossible to commit"),
                              tr("There are files with conflicts. Please, resolve "
                                 "the conflicts first."));
      else if (checkMsg(msg))
      {
         const auto revInfo = mCache->commitInfo(CommitInfo::ZERO_SHA);

         QScopedPointer<GitWip> git(new GitWip(mGit, mCache));
         git->updateWip();

         if (const auto files = mCache->revisionFile(CommitInfo::ZERO_SHA, revInfo.firstParent()); files)
         {
            const auto lastShaBeforeCommit = mGit->getLastCommit().output.trimmed();
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            QScopedPointer<GitLocal> gitLocal(new GitLocal(mGit));
            const auto ret = gitLocal->commitFiles(selFiles, files.value(), msg);
            QApplication::restoreOverrideCursor();

            if (ret.success)
            {
               // Adding new commit in the log
               const auto currentSha = mGit->getLastCommit().output.trimmed();
               QScopedPointer<GitConfig> gitConfig(new GitConfig(mGit));
               auto committer = gitConfig->getLocalUserInfo();

               if (committer.mUserEmail.isEmpty() || committer.mUserName.isEmpty())
                  committer = gitConfig->getGlobalUserInfo();

               const auto message = msg.split("\n\n");

               CommitInfo newCommit { currentSha,
                                      { lastShaBeforeCommit },
                                      std::chrono::seconds(QDateTime::currentDateTime().toSecsSinceEpoch()),
                                      ui->leCommitTitle->text() };

               newCommit.committer = QString("%1<%2>").arg(committer.mUserName, committer.mUserEmail);
               newCommit.author = QString("%1<%2>").arg(committer.mUserName, committer.mUserEmail);
               newCommit.longLog = ui->teDescription->toPlainText();

               mCache->insertCommit(newCommit);
               mCache->deleteReference(lastShaBeforeCommit, References::Type::LocalBranch, mGit->getCurrentBranch());
               mCache->insertReference(currentSha, References::Type::LocalBranch, mGit->getCurrentBranch());

               QScopedPointer<GitHistory> gitHistory(new GitHistory(mGit));
               const auto ret = gitHistory->getDiffFiles(currentSha, lastShaBeforeCommit);

               mCache->insertRevisionFiles(currentSha, lastShaBeforeCommit, RevisionFiles(ret.output));

               prepareCache();
               clearCache();

               ui->stagedFilesList->clear();
               ui->stagedFilesList->update();

               ui->leCommitTitle->clear();
               ui->teDescription->clear();

               git->updateWip();

               emit mCache->signalCacheUpdated();
               emit changesCommitted();
            }
            else
            {
               QMessageBox msgBox(QMessageBox::Critical, tr("Error when commiting"),
                                  tr("There were problems during the commit "
                                     "operation. Please, see the detailed "
                                     "description for more information."),
                                  QMessageBox::Ok, this);
               msgBox.setDetailedText(ret.output);
               msgBox.setStyleSheet(GitQlientStyles::getStyles());
               msgBox.exec();
            }

            lastMsgBeforeError = (ret.success ? "" : msg);
         }
      }
   }
}
