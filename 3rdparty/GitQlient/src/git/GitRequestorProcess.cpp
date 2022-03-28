#include "GitRequestorProcess.h"

#include <QTemporaryFile>
GitRequestorProcess::GitRequestorProcess(const QString &workingDir)
   : AGitProcess(workingDir)
{
}

GitExecResult GitRequestorProcess::run(const QString &command)
{
   auto ret = false;

   // Create temporary file
   mTempFile = new QTemporaryFile(this);

   if (mTempFile->open()) // to read the file name
   {
      setStandardOutputFile(mTempFile->fileName());
      mTempFile->close();

      ret = execute(command);
   }

   return { ret, "" };
}

void GitRequestorProcess::onFinished(int, QProcess::ExitStatus)
{
   bool ok = mTempFile && (mTempFile->isOpen() || (mTempFile->exists() && mTempFile->open()));

   if (ok && !mCanceling)
      emit procDataReady(mTempFile->readAll());

   deleteLater();
}
