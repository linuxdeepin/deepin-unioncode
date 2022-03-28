#include "GitBase.h"

#include <GitAsyncProcess.h>
#include <GitSyncProcess.h>

#include <QLogger.h>

using namespace QLogger;

#include <QDir>
#include <QFileInfo>

GitBase::GitBase(const QString &workingDirectory)
   : mWorkingDirectory(workingDirectory)
   , mGitDirectory(mWorkingDirectory + "/.git")
{
   QFileInfo fileInfo(mGitDirectory);

   if (fileInfo.isFile())
   {
      QFile f(fileInfo.filePath());

      if (f.open(QIODevice::ReadOnly))
      {
         auto path = f.readAll().split(':').last().trimmed();
         mGitDirectory = mWorkingDirectory + "/" + path;
         f.close();
      }
   }
}

QString GitBase::getWorkingDir() const
{
   return mWorkingDirectory;
}

void GitBase::setWorkingDir(const QString &workingDir)
{
   mWorkingDirectory = workingDir;
}

QString GitBase::getGitDir() const
{
   return mGitDirectory;
}

GitExecResult GitBase::run(const QString &cmd) const
{
   GitSyncProcess p(mWorkingDirectory);

   const auto ret = p.run(cmd);
   const auto runOutput = ret.output;

   if (ret.success && runOutput.contains("fatal:"))
      QLog_Info("Git", QString("Git command {%1} reported issues:\n%2").arg(cmd, runOutput));
   else if (!ret.success)
      QLog_Warning("Git", QString("Git command {%1} has errors:\n%2").arg(cmd, runOutput));

   return ret;
}

void GitBase::updateCurrentBranch()
{
   QLog_Trace("Git", "Updating the cached current branch");

   const auto cmd = QString("git rev-parse --abbrev-ref HEAD");

   QLog_Trace("Git", QString("Updating the cached current branch: {%1}").arg(cmd));

   const auto ret = run(cmd);

   mCurrentBranch = ret.success ? ret.output.trimmed().remove("heads/") : QString();
}

QString GitBase::getCurrentBranch()
{
   if (mCurrentBranch.isEmpty())
      updateCurrentBranch();

   return mCurrentBranch;
}

GitExecResult GitBase::getLastCommit() const
{
   QLog_Trace("Git", "Getting last commit");

   const auto cmd = QString("git rev-parse HEAD");

   QLog_Trace("Git", QString("Getting last commit: {%1}").arg(cmd));

   const auto ret = run(cmd);

   return ret;
}
