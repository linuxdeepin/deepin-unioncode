#include "StashesContextMenu.h"

#include <BranchDlg.h>
#include <GitQlientStyles.h>
#include <GitStashes.h>

#include <QMessageBox>

StashesContextMenu::StashesContextMenu(const QSharedPointer<GitBase> &git, const QString &stashId, QWidget *parent)
   : QMenu(parent)
   , mGit(git)
   , mStashId(stashId)
{
   setAttribute(Qt::WA_DeleteOnClose);

   connect(addAction(tr("Branch")), &QAction::triggered, this, &StashesContextMenu::branch);
   connect(addAction(tr("Drop")), &QAction::triggered, this, &StashesContextMenu::drop);
   connect(addAction(tr("Clear all")), &QAction::triggered, this, &StashesContextMenu::clear);
}

void StashesContextMenu::branch()
{
   BranchDlg dlg({ mStashId, BranchDlgMode::STASH_BRANCH, nullptr, mGit });
   const auto ret = dlg.exec();

   if (ret == QDialog::Accepted)
      emit signalUpdateView();
}

void StashesContextMenu::drop()
{
   QScopedPointer<GitStashes> git(new GitStashes(mGit));
   const auto ret = git->stashDrop(mStashId);

   if (ret.success)
      emit signalUpdateView();
   else
   {
      QMessageBox msgBox(QMessageBox::Critical, tr("Error while dropping stash"),
                         tr("There were problems during the stash drop operation. Please, see the detailed "
                            "description for more information."),
                         QMessageBox::Ok, this);
      msgBox.setDetailedText(ret.output);
      msgBox.setStyleSheet(GitQlientStyles::getStyles());
      msgBox.exec();
   }
}

void StashesContextMenu::clear()
{
   QScopedPointer<GitStashes> git(new GitStashes(mGit));
   const auto ret = git->stashClear();

   if (ret.success)
      emit signalUpdateView();
   else
   {
      QMessageBox msgBox(QMessageBox::Critical, tr("Error while branch stash"),
                         tr("There were problems during the branch stash operation. Please, see the detailed "
                            "description for more information."),
                         QMessageBox::Ok, this);
      msgBox.setDetailedText(ret.output);
      msgBox.setStyleSheet(GitQlientStyles::getStyles());
      msgBox.exec();
   }
}
