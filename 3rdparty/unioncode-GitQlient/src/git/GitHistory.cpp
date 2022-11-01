#include "GitHistory.h"

#include <CommitInfo.h>
#include <GitBase.h>
#include <GitConfig.h>

#include <QLogger.h>

#include <QStringLiteral>

using namespace QLogger;

GitHistory::GitHistory(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

GitExecResult GitHistory::blame(const QString &file, const QString &commitFrom)
{
   QLog_Debug("Git", QString("Executing blame: {%1} from {%2}").arg(file, commitFrom));

   const auto cmd = QString("git annotate %1 %2").arg(file, commitFrom);

   QLog_Trace("Git", QString("Executing blame: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitHistory::history(const QString &file)
{
   QLog_Debug("Git", QString("Executing history: {%1}").arg(file));

   const auto cmd = QString("git log --follow --pretty=%H %1").arg(file);

   QLog_Trace("Git", QString("Executing history: {%1}").arg(cmd));

   auto ret = mGitBase->run(cmd);

   if (ret.success && ret.output.isEmpty())
      ret.success = false;

   return ret;
}

GitExecResult GitHistory::getBranchesDiff(const QString &base, const QString &head)
{
   QLog_Debug("Git", QString("Getting diff between branches: {%1} and {%2}").arg(base, head));

   QScopedPointer<GitConfig> git(new GitConfig(mGitBase));

   QString fullBase = base;
   auto retBase = git->getRemoteForBranch(base);

   if (retBase.success)
      fullBase.prepend(retBase.output + QStringLiteral("/"));

   QString fullHead = head;
   auto retHead = git->getRemoteForBranch(head);

   if (retHead.success)
      fullHead.prepend(retHead.output + QStringLiteral("/"));

   const auto cmd = QString("git diff %1...%2").arg(fullBase, fullHead);

   QLog_Trace("Git", QString("Getting diff between branches: {%1}").arg(cmd));

   return mGitBase->run(cmd);
}

GitExecResult GitHistory::getCommitDiff(const QString &sha, const QString &diffToSha)
{
   if (!sha.isEmpty())
   {
      QLog_Debug("Git", QString("Executing diff for commit: {%1} to {%2}").arg(sha, diffToSha));

      QString runCmd = QString("git diff-tree --no-color -r --patch-with-stat -m");

      if (sha != CommitInfo::ZERO_SHA)
      {
         runCmd += " -C ";

         if (diffToSha.isEmpty())
            runCmd += " --root ";

         runCmd.append(QString("%1 %2").arg(diffToSha, sha)); // diffToSha could be empty
      }
      else
         runCmd = "git diff HEAD ";

      QLog_Trace("Git", QString("Executing diff for commit: {%1}").arg(runCmd));

      return mGitBase->run(runCmd);
   }
   else
      QLog_Warning("Git", QString("Executing getCommitDiff with empty SHA"));

   return qMakePair(false, QString());
}

GitExecResult GitHistory::getFileDiff(const QString &currentSha, const QString &previousSha, const QString &file,
                                      bool isCached)
{
   QLog_Debug("Git", QString("Getting diff for a file: {%1} between {%2} and {%3}").arg(file, currentSha, previousSha));

   auto cmd = QString("git diff %1 -w -U15000 ").arg(QString::fromUtf8(isCached ? "--cached" : ""));

   if (currentSha.isEmpty() || currentSha == CommitInfo::ZERO_SHA)
      cmd.append(file);
   else
      cmd.append(QString("%1 %2 %3").arg(previousSha, currentSha, file));

   QLog_Trace("Git", QString("Getting diff for a file: {%1}").arg(cmd));

   return mGitBase->run(cmd);
}

GitExecResult GitHistory::getDiffFiles(const QString &sha, const QString &diffToSha)
{
   QLog_Debug("Git", QString("Getting modified files between SHAs: {%1} to {%2}").arg(sha, diffToSha));

   auto runCmd = QString("git diff-tree -C --no-color -r -m ");

   if (!diffToSha.isEmpty() && sha != CommitInfo::ZERO_SHA)
      runCmd.append(diffToSha + " " + sha);
   else
      runCmd.append("4b825dc642cb6eb9a060e54bf8d69288fbee4904 " + sha);

   QLog_Trace("Git", QString("Getting modified files between SHAs: {%1}").arg(runCmd));

   return mGitBase->run(runCmd);
}

GitExecResult GitHistory::getUntrackedFileDiff(const QString &file) const
{
   QLog_Debug("Git", QString("Getting diff for untracked file {%1}").arg(file));

   auto cmd = QString("git add --intent-to-add %1").arg(file);

   QLog_Trace("Git", QString("Simulating we stage the file: {%1}").arg(cmd));

   if (auto ret = mGitBase->run(cmd); ret.success)
   {
      cmd = QString("git diff %1").arg(file);

      QLog_Trace("Git", QString("Getting diff for untracked file: {%1}").arg(cmd));

      const auto retDiff = mGitBase->run(cmd);

      QLog_Trace("Git", QString("Resetting the file to its previous state: {%1}").arg(cmd));

      cmd = QString("git reset %1").arg(file);

      mGitBase->run(cmd);

      return { true, retDiff.output };
   }
   else
      return { false, "" };
}
