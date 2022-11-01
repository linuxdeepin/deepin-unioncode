#include "GitAsyncProcess.h"

GitAsyncProcess::GitAsyncProcess(const QString &workingDir)
   : AGitProcess(workingDir)
{
}

GitExecResult GitAsyncProcess::run(const QString &command)
{
   const auto ret = execute(command);

   return { ret, "" };
}

void GitAsyncProcess::onFinished(int code, QProcess::ExitStatus exitStatus)
{
   AGitProcess::onFinished(code, exitStatus);

   if (!mCanceling)
      emit signalDataReady({ !mRealError, mRunOutput });

   deleteLater();
}
