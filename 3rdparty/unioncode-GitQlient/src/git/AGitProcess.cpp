#include "AGitProcess.h"

#include <GitQlientSettings.h>
#include <QTemporaryFile>
#include <QTextStream>

#include <QLogger.h>

using namespace QLogger;

namespace
{
QString loginApp()
{
   const auto askPassApp = qEnvironmentVariable("SSH_ASKPASS");

   if (!askPassApp.isEmpty())
      return QString("%1=%2").arg("SSH_ASKPASS", askPassApp);

#if defined(Q_OS_WIN)
   return QString("SSH_ASKPASS=win-ssh-askpass");
#else
   return QString("SSH_ASKPASS=ssh-askpass");
#endif
}

void restoreSpaces(QString &newCmd, const QChar &sepChar)
{
   QChar quoteChar;
   auto replace = false;
   const auto newCommandLength = newCmd.length();

   for (int i = 0; i < newCommandLength; ++i)
   {
      const auto c = newCmd[i];

      if (!replace && (c == "$"[0] || c == '\"' || c == '\'') && (newCmd.count(c) % 2 == 0))
      {
         replace = true;
         quoteChar = c;
         continue;
      }

      if (replace && (c == quoteChar))
      {
         replace = false;
         continue;
      }

      if (replace && c == sepChar)
         newCmd[i] = QChar(' ');
   }
}

QStringList splitArgList(const QString &cmd)
{
   // return argument list handling quotes and double quotes
   // substring, as example from:
   // cmd some_arg "some thing" v='some value'
   // to (comma separated fields)
   // sl = <cmd,some_arg,some thing,v='some value'>

   // early exit the common case
   if (!(cmd.contains("$") || cmd.contains("\"") || cmd.contains("\'")))
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
      return cmd.split(' ', Qt::SkipEmptyParts);
#else
      return cmd.split(' ', QString::SkipEmptyParts);
#endif

   // we have some work to do...
   // first find a possible separator
   const QString sepList("#%&!?"); // separator candidates
   int i = 0;
   while (cmd.contains(sepList[i]) && i < sepList.length())
      i++;

   if (i == sepList.length())
      return QStringList();

   const QChar &sepChar(sepList[i]);

   // remove all spaces
   QString newCmd(cmd);
   newCmd.replace(QChar(' '), sepChar);

   // re-add spaces in quoted sections
   restoreSpaces(newCmd, sepChar);

   // "$" is used internally to delimit arguments
   // with quoted text wholly inside as
   // arg1 = <[patch] cool patch on "cool feature">
   // and should be removed before to feed QProcess
   newCmd.remove("$");

   // QProcess::setArguments doesn't want quote
   // delimited arguments, so remove trailing quotes

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
   auto sl = QStringList(newCmd.split(sepChar, Qt::SkipEmptyParts));
#else
   auto sl = QStringList(newCmd.split(sepChar, QString::SkipEmptyParts));
#endif
   QStringList::iterator it(sl.begin());

   for (; it != sl.end(); ++it)
   {
      if (it->isEmpty())
         continue;

      if (((*it).at(0) == "\"" && (*it).right(1) == "\"") || ((*it).at(0) == "\'" && (*it).right(1) == "\'"))
         *it = (*it).mid(1, (*it).length() - 2);
   }
   return sl;
}
}

AGitProcess::AGitProcess(const QString &workingDir)
   : mWorkingDirectory(workingDir)
{
   setWorkingDirectory(mWorkingDirectory);

   connect(this, &AGitProcess::readyReadStandardOutput, this, &AGitProcess::onReadyStandardOutput,
           Qt::DirectConnection);
   connect(this, static_cast<void (AGitProcess::*)(int, QProcess::ExitStatus)>(&AGitProcess::finished), this,
           &AGitProcess::onFinished, Qt::DirectConnection);
}

void AGitProcess::onCancel()
{

   mCanceling = true;

   waitForFinished();
}

void AGitProcess::onReadyStandardOutput()
{
   if (!mCanceling)
   {
      const auto standardOutput = readAllStandardOutput();

      mRunOutput.append(QString::fromUtf8(standardOutput));

      emit procDataReady(standardOutput);
   }
}

bool AGitProcess::execute(const QString &command)
{
   mCommand = command;

   auto processStarted = false;
   auto arguments = splitArgList(mCommand);

   if (!arguments.isEmpty())
   {
      QStringList env = QProcess::systemEnvironment();
      env << "GIT_TRACE=0"; // avoid choking on debug traces
      env << "GIT_FLUSH=0"; // skip the fflush() in 'git log'
      env << loginApp();

      const auto gitAlternative = GitQlientSettings().globalValue("gitLocation", "").toString();

      setEnvironment(env);
      setProgram(gitAlternative.isEmpty() ? arguments.takeFirst() : gitAlternative);
      setArguments(arguments);
      start();

      processStarted = waitForStarted();

      if (!processStarted)
         QLog_Warning("Git", QString("Unable to start the process:\n%1\nMore info:\n%2").arg(mCommand, errorString()));
      else
         QLog_Debug("Git", QString("Process started: %1").arg(mCommand));
   }

   return processStarted;
}

void AGitProcess::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
   Q_UNUSED(exitCode)

   QLog_Debug("Git", QString("Process {%1} finished.").arg(mCommand));

   const auto errorOutput = readAllStandardError();

   mErrorOutput = QString::fromUtf8(errorOutput);
   mRealError = exitStatus != QProcess::NormalExit || mCanceling || errorOutput.contains("error") || errorOutput.contains("fatal: ")
       || errorOutput.toLower().contains("could not read username");

   if (mRealError)
   {
      if (!mErrorOutput.isEmpty())
         mRunOutput = mErrorOutput;
   }
   else
      mRunOutput.append(readAllStandardOutput() + mErrorOutput);
}
