#include "GitMerge.h"

#include <GitBase.h>
#include <GitRepoLoader.h>
#include <GitWip.h>
#include <QLogger.h>

#include <QFile>

using namespace QLogger;

GitMerge::GitMerge(const QSharedPointer<GitBase> &gitBase, QSharedPointer<GitCache> cache)
   : mGitBase(gitBase)
   , mCache(cache)
{
}

bool GitMerge::isInMerge() const
{
   QFile mergeHead(QString("%1/MERGE_HEAD").arg(mGitBase->getGitDir()));

   return mergeHead.exists();
}

GitExecResult GitMerge::merge(const QString &into, QStringList sources)
{
   QLog_Debug("Git", QString("Executing merge: {%1} into {%2}").arg(sources.join(","), into));

   {
      const auto cmd = QString("git checkout -q %1").arg(into);

      QLog_Trace("Git", QString("Checking out the current branch: {%1}").arg(cmd));

      const auto retCheckout = mGitBase->run(cmd);

      if (!retCheckout.success)
         return retCheckout;
   }

   const auto cmd2 = QString("git merge -Xignore-all-space ").append(sources.join(" "));

   QLog_Trace("Git", QString("Merging ignoring spaces: {%1}").arg(cmd2));

   const auto retMerge = mGitBase->run(cmd2);

   if (retMerge.success)
   {
      QScopedPointer<GitWip> git(new GitWip(mGitBase, mCache));
      git->updateWip();
   }

   return retMerge;
}

GitExecResult GitMerge::abortMerge() const
{
   QLog_Debug("Git", QString("Aborting merge"));

   const auto cmd = QString("git merge --abort");

   QLog_Trace("Git", QString("Aborting merge: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitMerge::applyMerge() const
{
   QLog_Debug("Git", QString("Commiting merge"));

   const auto cmd = QString("git commit --no-edit");

   QLog_Trace("Git", QString("Commiting merge: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitMerge::squashMerge(const QString &into, QStringList sources, const QString &msg) const
{
   QLog_Debug("Git", QString("Executing squash merge: {%1} into {%2}").arg(sources.join(","), into));

   {
      const auto cmd = QString("git checkout -q %1").arg(into);

      QLog_Trace("Git", QString("Checking out the current branch: {%1}").arg(cmd));

      const auto retCheckout = mGitBase->run(cmd);

      if (!retCheckout.success)
         return retCheckout;
   }

   const auto cmd2 = QString("git merge  -Xignore-all-space --squash ").append(sources.join(" "));

   const auto retMerge = mGitBase->run(cmd2);

   if (retMerge.success)
   {
      if (msg.isEmpty())
      {
         const auto commitCmd = QString("git commit --no-edit");
         mGitBase->run(commitCmd);
      }
      else
      {
         const auto cmd = QString("git commit -m \"%1\"").arg(msg);
         mGitBase->run(cmd);
      }

      QScopedPointer<GitWip> git(new GitWip(mGitBase, mCache));
      git->updateWip();
   }

   return retMerge;
}
