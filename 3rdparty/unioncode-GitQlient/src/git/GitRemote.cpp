#include "GitRemote.h"

#include <GitBase.h>
#include <GitConfig.h>
#include <GitQlientSettings.h>
#include <GitSubmodules.h>

#include <QLogger.h>

using namespace QLogger;

GitRemote::GitRemote(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

GitExecResult GitRemote::pushBranch(const QString &branchName, bool force)
{
   QLog_Debug("Git", QString("Executing push"));

   QScopedPointer<GitConfig> gitConfig(new GitConfig(mGitBase));
   auto ret = gitConfig->getRemoteForBranch(branchName);

   if (ret.success)
   {
      const auto remote = ret.output.isEmpty() ? QString("origin") : ret.output;
      ret = mGitBase->run(QString("git push %1 %2 %3").arg(remote, branchName, force ? QString("--force") : QString()));
   }

   return ret;
}

GitExecResult GitRemote::push(bool force)
{
   QLog_Debug("Git", QString("Executing push"));

   const auto ret = mGitBase->run(QString("git push ").append(force ? QString("--force") : QString()));

   return ret;
}

GitExecResult GitRemote::pushCommit(const QString &sha, const QString &remoteBranch)
{
   QLog_Debug("Git", QString("Executing pushCommit"));

   QScopedPointer<GitConfig> gitConfig(new GitConfig(mGitBase));
   const auto remote = gitConfig->getRemoteForBranch(remoteBranch);

   return mGitBase->run(QString("git push %1 %2:refs/heads/%3")
                            .arg(remote.success ? remote.output : QString("origin"), sha, remoteBranch));
}

GitExecResult GitRemote::pull()
{
   QLog_Debug("Git", QString("Executing pull"));

   auto ret = mGitBase->run("git pull --ff-only");

   GitQlientSettings settings(mGitBase->getGitDir());
   const auto updateOnPull = settings.localValue("UpdateOnPull", true).toBool();

   if (ret.success && updateOnPull)
   {
      QScopedPointer<GitSubmodules> git(new GitSubmodules(mGitBase));
      const auto updateRet = git->submoduleUpdate(QString());

      if (!updateRet)
      {
         return { updateRet,
                  "There was a problem updating the submodules after pull. Please review that you don't have any local "
                  "modifications in the submodules" };
      }
   }

   return ret;
}

bool GitRemote::fetch()
{
   QLog_Debug("Git", QString("Executing fetch with prune"));

   GitQlientSettings settings(mGitBase->getGitDir());
   const auto pruneOnFetch = settings.localValue("PruneOnFetch", true).toBool();

   const auto cmd
       = QString("git fetch --all --tags --force %1").arg(pruneOnFetch ? QString("--prune --prune-tags") : QString());
   const auto ret = mGitBase->run(cmd).success;

   return ret;
}

GitExecResult GitRemote::prune()
{
   QLog_Debug("Git", QString("Executing prune"));

   const auto ret = mGitBase->run("git remote prune origin");

   return ret;
}

GitExecResult GitRemote::addRemote(const QString &remoteRepo, const QString &remoteName)
{
   QLog_Debug("Git", QString("Adding a remote repository"));

   const auto ret = mGitBase->run(QString("git remote add %1 %2").arg(remoteName, remoteRepo));

   if (ret.success)
   {
      const auto ret2 = mGitBase->run(QString("git fetch %1").arg(remoteName));
   }

   return ret;
}

GitExecResult GitRemote::removeRemote(const QString &remoteName)
{
   QLog_Debug("Git", QString("Removing a remote repository"));

   return mGitBase->run(QString("git remote rm %1").arg(remoteName));
}
