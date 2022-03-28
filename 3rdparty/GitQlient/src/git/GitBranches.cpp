#include "GitBranches.h"

#include <GitBase.h>
#include <GitConfig.h>

#include <QLogger.h>

using namespace QLogger;

GitBranches::GitBranches(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

GitExecResult GitBranches::createBranchFromAnotherBranch(const QString &oldName, const QString &newName)
{
   QLog_Debug("Git", QString("Creating branch from another branch: {%1} and {%2}").arg(oldName, newName));

   const auto cmd = QString("git branch %1 %2").arg(newName, oldName);

   QLog_Trace("Git", QString("Creating branch from another branch: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitBranches::createBranchAtCommit(const QString &commitSha, const QString &branchName)
{
   QLog_Debug("Git", QString("Creating a branch from a commit: {%1} at {%2}").arg(branchName, commitSha));

   const auto cmd = QString("git branch %1 %2").arg(branchName, commitSha);

   QLog_Trace("Git", QString("Creating a branch from a commit: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitBranches::checkoutBranchFromCommit(const QString &commitSha, const QString &branchName)
{
   QLog_Debug("Git",
              QString("Creating and checking out a branch from a commit: {%1} at {%2}").arg(branchName, commitSha));

   const auto cmd = QString("git checkout -b %1 %2").arg(branchName, commitSha);

   QLog_Trace("Git", QString("Creating and checking out a branch from a commit: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   if (ret.success)
      mGitBase->updateCurrentBranch();

   return ret;
}

GitExecResult GitBranches::checkoutLocalBranch(const QString &branchName)
{
   QLog_Debug("Git", QString("Checking out local branch: {%1}").arg(branchName));

   const auto cmd = QString("git checkout %1").arg(branchName);

   QLog_Trace("Git", QString("Checking out local branch: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   if (ret.success)
      mGitBase->updateCurrentBranch();

   return ret;
}

GitExecResult GitBranches::checkoutRemoteBranch(const QString &branchName)
{
   QLog_Debug("Git", QString("Checking out remote branch: {%1}").arg(branchName));

   auto localBranch = branchName;
   if (localBranch.startsWith("origin/"))
      localBranch.remove("origin/");

   const auto cmd = QString("git checkout -b %1 %2").arg(localBranch, branchName);

   QLog_Trace("Git", QString("Checking out remote branch: {%1}").arg(cmd));

   auto ret = mGitBase->run(cmd);
   const auto output = ret.output;

   if (ret.success && !output.contains("fatal:"))
      mGitBase->updateCurrentBranch();
   else if (output.contains("already exists"))
   {
      QRegExp rx("\'\\w+\'");
      rx.indexIn(ret.output);
      auto value = rx.capturedTexts().constFirst();
      value.remove("'");

      if (!value.isEmpty())
         ret = checkoutLocalBranch(value);
      else
         ret.success = false;
   }

   return ret;
}

GitExecResult GitBranches::checkoutNewLocalBranch(const QString &branchName)
{
   QLog_Debug("Git", QString("Checking out new local branch: {%1}").arg(branchName));

   const auto cmd = QString("git checkout -b %1").arg(branchName);

   QLog_Trace("Git", QString("Checking out new local branch: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   if (ret.success)
      mGitBase->updateCurrentBranch();

   return ret;
}

GitExecResult GitBranches::renameBranch(const QString &oldName, const QString &newName)
{
   QLog_Debug("Git", QString("Renaming branch: {%1} at {%2}").arg(oldName, newName));

   const auto cmd = QString("git branch -m %1 %2").arg(oldName, newName);

   QLog_Trace("Git", QString("Renaming branch: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   if (ret.success)
      mGitBase->updateCurrentBranch();

   return ret;
}

GitExecResult GitBranches::removeLocalBranch(const QString &branchName)
{
   QLog_Debug("Git", QString("Removing local branch: {%1}").arg(branchName));

   const auto cmd = QString("git branch -D %1").arg(branchName);

   QLog_Trace("Git", QString("Removing local branch: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitBranches::removeRemoteBranch(const QString &branchName)
{
   auto branch = branchName;
   branch = branch.mid(branch.indexOf('/') + 1);

   QLog_Debug("Git", QString("Removing a remote branch: {%1}").arg(branch));

   QScopedPointer<GitConfig> gitConfig(new GitConfig(mGitBase));

   auto ret = gitConfig->getRemoteForBranch(branch);

   const auto cmd = QString("git push --delete %2 %1").arg(branch, ret.success ? ret.output : QString("origin"));

   QLog_Trace("Git", QString("Removing a remote branch: {%1}").arg(cmd));

   ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitBranches::getLastCommitOfBranch(const QString &branch)
{
   QLog_Debug("Git", QString("Getting last commit of a branch: {%1}").arg(branch));

   const auto cmd = QString("git rev-parse %1").arg(branch);

   QLog_Trace("Git", QString("Getting last commit of a branch: {%1}").arg(cmd));

   auto ret = mGitBase->run(cmd);

   if (ret.success)
      ret.output = ret.output.trimmed();

   return ret;
}

GitExecResult GitBranches::pushUpstream(const QString &branchName)
{
   QLog_Debug("Git", QString("Pushing upstream: {%1}").arg(branchName));

   const auto cmd = QString("git push --set-upstream origin %1").arg(branchName);

   QLog_Trace("Git", QString("Pushing upstream: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitBranches::rebaseOnto(const QString &currentBranch, const QString &startBranch,
                                      const QString &fromBranch) const
{
   QLog_Debug("Git", QString("Git rebase {%1} into {%2}").arg(currentBranch, fromBranch));

   const auto cmd = QString("git rebase --onto %1 %2 %3").arg(currentBranch, startBranch, fromBranch);
   return mGitBase->run(cmd);
}
