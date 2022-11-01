#include "UnstagedMenu.h"

#include <GitBase.h>
#include <GitLocal.h>
#include <GitSyncProcess.h>
#include <QLogger.h>

#include <QDir>
#include <QFile>
#include <QMessageBox>

using namespace QLogger;

UnstagedMenu::UnstagedMenu(const QSharedPointer<GitBase> &git, const QString &fileName, QWidget *parent)
   : QMenu(parent)
   , mGit(git)
   , mFileName(fileName)
{
   setAttribute(Qt::WA_DeleteOnClose);

   connect(addAction(tr("See changes")), &QAction::triggered, this, [this]() { emit signalShowDiff(mFileName); });
   connect(addAction(tr("Blame")), &QAction::triggered, this, [this]() { emit signalShowFileHistory(mFileName); });
   connect(addAction(tr("Edit file")), &QAction::triggered, this,
           [this]() { emit signalEditFile(mGit->getWorkingDir() + "/" + mFileName); });

   addSeparator();

   connect(addAction(tr("Stage file")), &QAction::triggered, this, &UnstagedMenu::signalStageFile);

   connect(addAction(tr("Revert file changes")), &QAction::triggered, this, [this]() {
      const auto msgBoxRet
          = QMessageBox::question(this, tr("Ignoring file"), tr("Are you sure you want to revert the changes?"));

      if (msgBoxRet == QMessageBox::Yes)
      {
         QScopedPointer<GitLocal> git(new GitLocal(mGit));
         const auto ret = git->checkoutFile(mFileName);

         if (ret)
            emit changeReverted(mFileName);
      }
   });

   addSeparator();

   const auto ignoreMenu = addMenu(tr("Ignore"));

   connect(ignoreMenu->addAction(tr("Ignore file")), &QAction::triggered, this, [this]() {
      const auto ret = QMessageBox::question(this, tr("Ignoring file"),
                                             tr("Are you sure you want to add the file to the black list?"));

      if (ret == QMessageBox::Yes)
      {
         const auto gitRet = addEntryToGitIgnore(mFileName);

         if (gitRet)
            emit signalCheckedOut();
      }
   });

   connect(addAction(tr("Delete file")), &QAction::triggered, this, &UnstagedMenu::onDeleteFile);

   connect(addAction(tr("Delete ALL untracked files")), &QAction::triggered, this, &UnstagedMenu::deleteUntracked);

   connect(ignoreMenu->addAction(tr("Ignore extension")), &QAction::triggered, this, [this]() {
      const auto msgBoxRet = QMessageBox::question(
          this, tr("Ignoring extension"), tr("Are you sure you want to add the file extension to the black list?"));

      if (msgBoxRet == QMessageBox::Yes)
      {
         auto fileParts = mFileName.split(".");
         fileParts.takeFirst();
         const auto extension = QString("*.%1").arg(fileParts.join("."));
         const auto ret = addEntryToGitIgnore(extension);

         if (ret)
            emit signalCheckedOut();
      }
   });

   connect(ignoreMenu->addAction(tr("Ignore containing folder")), &QAction::triggered, this, [this]() {
      const auto msgBoxRet = QMessageBox::question(
          this, tr("Ignoring folder"), tr("Are you sure you want to add the containing folder to the black list?"));

      if (msgBoxRet == QMessageBox::Yes)
      {
         const auto path = mFileName.lastIndexOf("/");
         const auto folder = mFileName.left(path);
         const auto extension = QString("%1/*").arg(folder);
         const auto ret = addEntryToGitIgnore(extension);

         if (ret)
            emit signalCheckedOut();
      }
   });

   addSeparator();

   connect(addAction(tr("Add all files to commit")), &QAction::triggered, this, &UnstagedMenu::signalCommitAll);
   connect(addAction(tr("Revert all changes")), &QAction::triggered, this, [this]() {
      const auto msgBoxRet
          = QMessageBox::question(this, tr("Ignoring file"), tr("Are you sure you want to undo all the changes?"));
      if (msgBoxRet == QMessageBox::Yes)
         emit signalRevertAll();
   });
}

bool UnstagedMenu::addEntryToGitIgnore(const QString &entry)
{
   auto entryAdded = false;
   QDir d(mGit->getWorkingDir());
   QFile f(d.absolutePath() + "/.gitignore");

   if (!f.exists())
   {
      if (f.open(QIODevice::ReadWrite))
         f.close();
   }

   if (f.open(QIODevice::Append))
   {
      const auto bytesWritten = f.write(entry.toUtf8() + "\n");

      if (bytesWritten != -1)
      {
         QMessageBox::information(this, tr("File added to .gitignore"),
                                  tr("The file has been added to the ignore list in the file .gitignore."));

         entryAdded = true;
      }
      else
         QMessageBox::critical(this, tr("Unable to add the entry"),
                               tr("It was impossible to add the entry in the .gitignore file."));

      f.close();
   }

   return entryAdded;
}

void UnstagedMenu::onDeleteFile()
{
   const auto path = QString("%1").arg(mFileName);

   QLog_Info("UI", "Removing path: " + path);

   QProcess p;
   p.setWorkingDirectory(mGit->getWorkingDir());
   p.start("rm", { "-rf", path });

   if (p.waitForFinished())
      emit signalCheckedOut();
}
