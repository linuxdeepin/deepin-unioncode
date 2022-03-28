#include "GitSyncProcess.h"

#include <QTemporaryFile>
#include <QTextStream>

GitSyncProcess::GitSyncProcess(const QString &workingDir)
   : AGitProcess(workingDir)
{
}

GitExecResult GitSyncProcess::run(const QString &command)
{
   const auto processStarted = execute(command);

   if (processStarted)
      waitForFinished(10000);

   close();

   return { !mRealError, mRunOutput };
}
