#include <AmendWidget.h>
#include <ui_CommitChangesWidget.h>

#include <GitBase.h>
#include <GitCache.h>
#include <GitHistory.h>
#include <GitLocal.h>
#include <GitQlientRole.h>
#include <GitQlientStyles.h>
#include <GitWip.h>
#include <UnstagedMenu.h>

#include <QMessageBox>

#include <QLogger.h>

using namespace QLogger;

AmendWidget::AmendWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git, QWidget *parent)
   : CommitChangesWidget(cache, git, parent)
{
   ui->applyActionBtn->setText(tr("Amend"));
}

void AmendWidget::configure(const QString &sha)
{
   const auto commit = mCache->commitInfo(sha);

   ui->amendFrame->setVisible(true);
   ui->warningButton->setVisible(true);

   if (commit.parentsCount() <= 0)
      return;

   QScopedPointer<GitWip> git(new GitWip(mGit, mCache));
   git->updateWip();

   const auto files = mCache->revisionFile(CommitInfo::ZERO_SHA, sha);
   auto amendFiles = mCache->revisionFile(sha, commit.firstParent());

   if (!amendFiles)
   {
      QScopedPointer<GitHistory> git(new GitHistory(mGit));
      const auto ret = git->getDiffFiles(mCurrentSha, commit.firstParent());

      if (ret.success)
      {
         amendFiles = RevisionFiles(ret.output);
         mCache->insertRevisionFiles(mCurrentSha, commit.firstParent(), amendFiles.value());
      }
   }

   if (mCurrentSha != sha)
   {
      QLog_Info("UI", QString("Amending sha {%1}.").arg(mCurrentSha));

      mCurrentSha = sha;

      const auto author = commit.author.split("<");
      ui->leAuthorName->setText(author.first());
      ui->leAuthorEmail->setText(author.last().mid(0, author.last().count() - 1));
      ui->teDescription->setPlainText(commit.longLog.trimmed());
      ui->leCommitTitle->setText(commit.shortLog);

      blockSignals(true);
      ui->unstagedFilesList->clear();
      ui->stagedFilesList->clear();
      mInternalCache.clear();
      blockSignals(false);

      if (files)
         insertFiles(files.value(), ui->unstagedFilesList);

      if (amendFiles)
         insertFiles(amendFiles.value(), ui->stagedFilesList);
   }
   else
   {
      QLog_Info("UI", QString("Updating files for SHA {%1}").arg(mCurrentSha));

      prepareCache();

      if (files)
         insertFiles(files.value(), ui->unstagedFilesList);

      clearCache();

      if (amendFiles)
         insertFiles(amendFiles.value(), ui->stagedFilesList);
   }

   ui->applyActionBtn->setEnabled(ui->stagedFilesList->count());
}

void AmendWidget::commitChanges()
{
   QStringList selFiles = getFiles();

   if (!selFiles.isEmpty())
   {
      QString msg;

      if (hasConflicts())
      {
         QMessageBox::critical(this, tr("Impossible to commit"),
                               tr("There are files with conflicts. Please, resolve the conflicts first."));
      }
      else if (checkMsg(msg))
      {
         QScopedPointer<GitWip> git(new GitWip(mGit, mCache));
         git->updateWip();

         const auto files = mCache->revisionFile(CommitInfo::ZERO_SHA, mCurrentSha);

         if (files)
         {
            const auto author = QString("%1<%2>").arg(ui->leAuthorName->text(), ui->leAuthorEmail->text());
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

            QScopedPointer<GitLocal> gitLocal(new GitLocal(mGit));
            const auto ret = gitLocal->ammendCommit(selFiles, files.value(), msg, author);
            QApplication::restoreOverrideCursor();

            emit logReload();

            if (ret.success)
            {
               const auto newSha = mGit->getLastCommit().output.trimmed();
               auto commit = mCache->commitInfo(mCurrentSha);
               const auto oldSha = commit.sha;
               commit.sha = newSha;
               commit.committer = author;
               commit.author = author;

               const auto log = msg.split("\n\n");
               commit.shortLog = log.constFirst();
               commit.longLog = log.constLast();

               mCache->updateCommit(oldSha, std::move(commit));

               QScopedPointer<GitHistory> git(new GitHistory(mGit));
               const auto ret = git->getDiffFiles(mCurrentSha, commit.firstParent());

               mCache->insertRevisionFiles(mCurrentSha, commit.firstParent(), RevisionFiles(ret.output));

               emit changesCommitted();
            }
            else
            {
               QMessageBox msgBox(QMessageBox::Critical, tr("Error when amending"),
                                  tr("There were problems during the commit "
                                     "operation. Please, see the detailed "
                                     "description for more information."),
                                  QMessageBox::Ok, this);
               msgBox.setDetailedText(ret.output);
               msgBox.setStyleSheet(GitQlientStyles::getStyles());
               msgBox.exec();
            }
         }
      }
   }
}
