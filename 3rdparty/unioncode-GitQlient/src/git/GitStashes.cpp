#include "GitStashes.h"

#include <GitBase.h>

#include <QLogger.h>

using namespace QLogger;

GitStashes::GitStashes(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

QVector<QString> GitStashes::getStashes()
{
   QLog_Debug("Git", QString("Getting stashes"));

   const auto cmd = QString("git stash list");

   QLog_Trace("Git", QString("Getting stashes: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   QVector<QString> stashes;

   if (ret.success)
   {
      const auto tagsTmp = ret.output.split("\n");

      for (const auto &tag : tagsTmp)
         if (tag != "\n" && !tag.isEmpty())
            stashes.append(tag);
   }

   return stashes;
}

GitExecResult GitStashes::pop() const
{
   QLog_Debug("Git", QString("Poping the stash"));

   const auto cmd = QString("git stash pop");

   QLog_Trace("Git", QString("Poping the stash: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitStashes::stash()
{
   QLog_Debug("Git", QString("Stashing changes"));

   const auto cmd = QString("git stash");

   QLog_Trace("Git", QString("Stashing changes: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitStashes::stashBranch(const QString &stashId, const QString &branchName)
{
   QLog_Debug("Git", QString("Creating a branch from stash: {%1} in branch {%2}").arg(stashId, branchName));

   const auto cmd = QString("git stash branch %1 %2").arg(branchName, stashId);

   QLog_Trace("Git", QString("Creating a branch from stash: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitStashes::stashDrop(const QString &stashId)
{
   QLog_Debug("Git", QString("Droping stash: {%1}").arg(stashId));

   const auto cmd = QString("git stash drop -q %1").arg(stashId);

   QLog_Trace("Git", QString("Droping stash: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitStashes::stashClear()
{
   QLog_Debug("Git", QString("Clearing stash"));

   const auto cmd = QString("git stash clear");

   QLog_Trace("Git", QString("Clearing stash: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}
