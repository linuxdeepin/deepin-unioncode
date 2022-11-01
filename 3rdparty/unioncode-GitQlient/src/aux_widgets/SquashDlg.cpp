#include "SquashDlg.h"
#include "ui_SquashDlg.h"

#include <CheckBox.h>
#include <GitBase.h>
#include <GitBranches.h>
#include <GitCache.h>
#include <GitLocal.h>
#include <GitMerge.h>
#include <GitQlientSettings.h>
#include <GitWip.h>

#include <QLabel>
#include <QMessageBox>
#include <QUuid>

SquashDlg::SquashDlg(const QSharedPointer<GitBase> git, const QSharedPointer<GitCache> &cache, const QStringList &shas,
                     QWidget *parent)
   : QDialog(parent)
   , mGit(git)
   , mCache(cache)
   , mShas(shas)
   , ui(new Ui::SquashDlg)
{
   ui->setupUi(this);

   setAttribute(Qt::WA_DeleteOnClose);

   mTitleMaxLength = GitQlientSettings().globalValue("commitTitleMaxLength", mTitleMaxLength).toInt();

   ui->lCounter->setText(QString::number(mTitleMaxLength));
   ui->leCommitTitle->setMaxLength(mTitleMaxLength);

   auto description = QString("This is a combination of %1 commits:\n\n").arg(shas.count());

   const auto commitsLayout = new QGridLayout();
   commitsLayout->setContentsMargins(10, 10, 10, 10);
   commitsLayout->setSpacing(10);
   commitsLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

   auto row = 0;
   for (const auto &sha : shas)
   {
      const auto shortSha = sha.left(8);
      const auto commitTitle = mCache->commitInfo(sha).shortLog;
      description.append(QString("Commit %1: %2 - %3\n\n").arg(row + 1).arg(shortSha, commitTitle));

      commitsLayout->addWidget(new QLabel(QString("<strong>(%1)</strong>").arg(shortSha)), row, 0);
      commitsLayout->addWidget(new QLabel(commitTitle), row, 1);
      ++row;
   }

   commitsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding), row, 0);

   ui->commitsFrame->setLayout(commitsLayout);
   ui->scrollArea->setWidgetResizable(true);
   ui->teDescription->setText(description);

   connect(ui->leCommitTitle, &QLineEdit::textChanged, this, &SquashDlg::updateCounter);
   connect(ui->leCommitTitle, &QLineEdit::returnPressed, this, &SquashDlg::accept);
}

SquashDlg::~SquashDlg()
{
   delete ui;
}

void SquashDlg::accept()
{
   QString msg;

   if (checkMsg(msg))
   {
      const auto revInfo = mCache->commitInfo(CommitInfo::ZERO_SHA);

      QScopedPointer<GitWip> git(new GitWip(mGit, mCache));
      git->updateWip();

      const auto lastChild = mCache->commitInfo(mShas.last());

      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

      if (lastChild.getChildsCount() == 1)
      {
         if (lastChild.isInWorkingBranch())
         {
            // Reset soft to the first commit to squash
            QScopedPointer<GitLocal> gitLocal(new GitLocal(mGit));
            gitLocal->resetCommit(mShas.constFirst(), GitLocal::CommitResetType::SOFT);
            gitLocal->ammend(msg);
         }
         else
         {
            QScopedPointer<GitBranches> gitBranches(new GitBranches(mGit));

            // Create auxiliar branch for rebase
            const auto auxBranch1 = QUuid::createUuid().toString();
            const auto commitOfAuxBranch1 = lastChild.getFirstChildSha();
            gitBranches->createBranchAtCommit(commitOfAuxBranch1, auxBranch1);

            // Create auxiliar branch for merge squash
            const auto auxBranch2 = QUuid::createUuid().toString();
            gitBranches->createBranchAtCommit(mShas.last(), auxBranch2);

            // Create auxiliar branch for final rebase
            const auto auxBranch3 = QUuid::createUuid().toString();
            const auto lastCommit = mCache->commitInfo(CommitInfo::ZERO_SHA).firstParent();
            gitBranches->createBranchAtCommit(lastCommit, auxBranch3);

            // Reset hard to the first commit to squash
            QScopedPointer<GitLocal> gitLocal(new GitLocal(mGit));
            gitLocal->resetCommit(mShas.constFirst(), GitLocal::CommitResetType::HARD);

            // Merge squash auxiliar branch 2
            QScopedPointer<GitMerge> gitMerge(new GitMerge(mGit, mCache));
            const auto ret = gitMerge->squashMerge(mGit->getCurrentBranch(), { auxBranch2 }, msg);

            gitBranches->removeLocalBranch(auxBranch2);

            // Rebase auxiliar branch 1
            const auto destBranch = mGit->getCurrentBranch();
            gitLocal->cherryPickCommit(commitOfAuxBranch1);
            gitBranches->rebaseOnto(destBranch, auxBranch1, auxBranch3);
            gitBranches->removeLocalBranch(auxBranch1);
            gitBranches->checkoutLocalBranch(destBranch);
            gitMerge->merge(destBranch, { auxBranch3 });
            gitBranches->removeLocalBranch(auxBranch3);
         }
      }

      QApplication::restoreOverrideCursor();

      emit changesCommitted();

      ui->leCommitTitle->clear();
      ui->teDescription->clear();

      QDialog::accept();
   }
}

void SquashDlg::updateCounter(const QString &text)
{
   ui->lCounter->setText(QString::number(mTitleMaxLength - text.count()));
}

bool SquashDlg::checkMsg(QString &msg)
{
   const auto title = ui->leCommitTitle->text();

   if (title.isEmpty())
   {
      QMessageBox::warning(this, "Commit changes", "Please, add a title.");
      return false;
   }

   msg = title;

   if (!ui->teDescription->toPlainText().isEmpty())
   {
      auto description = QString("\n\n%1").arg(ui->teDescription->toPlainText());
      description.remove(QRegExp("(^|\\n)\\s*#[^\\n]*")); // strip comments
      msg += description;
   }

   msg.replace(QRegExp("[ \\t\\r\\f\\v]+\\n"), "\n"); // strip line trailing cruft
   msg = msg.trimmed();

   if (msg.isEmpty())
   {
      QMessageBox::warning(this, "Commit changes", "Please, add a title.");
      return false;
   }

   QString subj(msg.section('\n', 0, 0, QString::SectionIncludeTrailingSep));
   QString body(msg.section('\n', 1).trimmed());
   msg = subj + '\n' + body + '\n';

   return true;
}
