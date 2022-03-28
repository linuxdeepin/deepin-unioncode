#include "GitLocal.h"

#include <GitBase.h>
#include <GitWip.h>
#include <QLogger.h>
#include <RevisionFiles.h>

#include <QFile>
#include <QProcess>

using namespace QLogger;

namespace
{
static QString quote(const QStringList &sl)
{
   QString q(sl.join(QString("$%1$").arg(' ')));
   q.prepend("$").append("$");
   return q;
}
}

GitLocal::GitLocal(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

GitExecResult GitLocal::stageFile(const QString &fileName) const
{
   QLog_Debug("Git", QString("Staging file: {%1}").arg(fileName));

   const auto cmd = QString("git add %1").arg(fileName);

   QLog_Trace("Git", QString("Staging file: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

bool GitLocal::isInCherryPickMerge() const
{
   QFile cherrypickHead(QString("%1/CHERRY_PICK_HEAD").arg(mGitBase->getGitDir()));

   return cherrypickHead.exists();
}

GitExecResult GitLocal::cherryPickCommit(const QString &sha) const
{
   QLog_Debug("Git", QString("Cherry-picking commit: {%1}").arg(sha));

   const auto cmd = QString(QString("git cherry-pick %1").arg(sha));

   QLog_Trace("Git", QString("Cherry-picking commit: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitLocal::cherryPickAbort() const
{
   QLog_Debug("Git", QString("Aborting cherryPick"));

   const auto cmd = QString("git cherry-pick --abort");

   QLog_Trace("Git", QString("Getting remote tags: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitLocal::cherryPickContinue() const
{
   QLog_Debug("Git", QString("Applying cherryPick"));

   const auto cmd = QString("git cherry-pick --continue");

   QLog_Trace("Git", QString("Applying cherryPick: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitLocal::checkoutCommit(const QString &sha) const
{
   QLog_Debug("Git", QString("Checking out a commit: {%1}").arg(sha));

   const auto cmd = QString("git checkout %1").arg(sha);

   QLog_Trace("Git", QString("Checking out a commit: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   if (ret.success)
      mGitBase->updateCurrentBranch();

   return ret;
}

GitExecResult GitLocal::markFilesAsResolved(const QStringList &files)
{
   QLog_Debug("Git", QString("Marking {%1} files as resolved").arg(files.count()));

   const auto cmd = QString("git add %1").arg(files.join(" "));

   QLog_Trace("Git", QString("Marking files as resolved: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

bool GitLocal::checkoutFile(const QString &fileName) const
{
   if (fileName.isEmpty())
   {
      QLog_Warning("Git", QString("Executing checkoutFile with an empty file.").arg(fileName));

      return false;
   }

   QLog_Debug("Git", QString("Checking out a file: {%1}").arg(fileName));

   const auto cmd = QString("git checkout %1").arg(fileName);

   QLog_Trace("Git", QString("Checking out a file: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd).success;

   return ret;
}

GitExecResult GitLocal::resetFile(const QString &fileName) const
{
   QLog_Debug("Git", QString("Resetting file: {%1}").arg(fileName));

   const auto cmd = QString("git reset %1").arg(fileName);

   QLog_Trace("Git", QString("Getting remote tags: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

bool GitLocal::resetCommit(const QString &sha, CommitResetType type)
{
   QString typeStr;

   switch (type)
   {
      case CommitResetType::SOFT:
         typeStr = "soft";
         break;
      case CommitResetType::MIXED:
         typeStr = "mixed";
         break;
      case CommitResetType::HARD:
         typeStr = "hard";
         break;
   }

   QLog_Debug("Git", QString("Reseting commit: {%1} type {%2}").arg(sha, typeStr));

   const auto cmd = QString("git reset --%1 %2").arg(typeStr, sha);

   QLog_Trace("Git", QString("Reseting commit: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret.success;
}

GitExecResult GitLocal::commit(const QString &msg) const
{
   QLog_Debug("Git", QString("Commit changes"));

   const auto cmd = QString("git commit -m \"%1\"").arg(msg);
   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitLocal::ammend(const QString &msg) const
{
   QLog_Debug("Git", QString("Amend message"));

   const auto cmd = QString("git commit --amend -m \"%1\"").arg(msg);
   return mGitBase->run(cmd);
}

GitExecResult GitLocal::commitFiles(QStringList &selFiles, const RevisionFiles &allCommitFiles,
                                    const QString &msg) const
{
   QStringList notSel;

   for (auto i = 0; i < allCommitFiles.count(); ++i)
   {
      if (const auto &fp = allCommitFiles.getFile(i);
          selFiles.indexOf(fp) == -1 && allCommitFiles.statusCmp(i, RevisionFiles::IN_INDEX))
      {
         notSel.append(fp);
      }
   }

   if (const auto updIdx = updateIndex(allCommitFiles, selFiles); !updIdx.success)
      return updIdx;

   QLog_Debug("Git", QString("Committing files"));

   const auto cmd = QString("git commit -m \"%1\"").arg(msg);

   QLog_Trace("Git", QString("Committing files: {%1}").arg(cmd));

   auto ret = mGitBase->run(cmd);

   if (ret.output.startsWith("On branch"))
      ret.output = false;

   return ret;
}

GitExecResult GitLocal::ammendCommit(const QStringList &selFiles, const RevisionFiles &allCommitFiles,
                                     const QString &msg, const QString &author) const
{
   QStringList notSel;

   for (auto i = 0; i < allCommitFiles.count(); ++i)
   {
      const QString &fp = allCommitFiles.getFile(i);
      if (selFiles.indexOf(fp) == -1 && allCommitFiles.statusCmp(i, RevisionFiles::IN_INDEX)
          && !allCommitFiles.statusCmp(i, RevisionFiles::DELETED))
         notSel.append(fp);
   }

   QLog_Debug("Git", QString("Amending files"));

   QString cmtOptions;

   if (!author.isEmpty())
      cmtOptions.append(QString(" --author \"%1\"").arg(author));

   const auto cmd = QString("git commit --amend" + cmtOptions + " -m \"%1\"").arg(msg);

   QLog_Trace("Git", QString("Amending files: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitLocal::updateIndex(const RevisionFiles &files, const QStringList &selFiles) const
{
   QStringList toRemove;

   for (const auto &file : selFiles)
   {
      const auto index = files.mFiles.indexOf(file);

      if (index != -1 && files.statusCmp(index, RevisionFiles::DELETED))
         toRemove << file;
   }

   if (!toRemove.isEmpty())
   {
      const auto cmd = QString("git rm --cached --ignore-unmatch -- " + quote(toRemove));

      QLog_Trace("Git", QString("Updating index for files: {%1}").arg(cmd));

      const auto ret = mGitBase->run(cmd);

      if (!ret.success)
      {

         return ret;
      }
   }

   const auto ret = GitExecResult(true, "Indexes updated");

   return ret;
}
