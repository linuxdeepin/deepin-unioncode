#include "GitPatches.h"

#include <GitBase.h>
#include <QLogger.h>

using namespace QLogger;

#include <QFile>

GitPatches::GitPatches(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

GitExecResult GitPatches::exportPatch(const QStringList &shaList)
{
   QLog_Debug("Git", QString("Executing exportPatch: {%1}").arg(shaList.join(",")));

   auto val = 1;
   QStringList files;

   for (const auto &sha : shaList)
   {
      const auto cmd = QString("git format-patch -1 %1").arg(sha);

      QLog_Trace("Git", QString("Exporting patch: {%1}").arg(cmd));

      const auto ret = mGitBase->run(cmd);

      if (!ret.success)
         break;
      else
      {
         auto filename = ret.output;
         filename = filename.remove("\n");
         const auto text = filename.mid(filename.indexOf("-") + 1);
         const auto number = QString("%1").arg(val, 4, 10, QChar('0'));
         const auto newFileName = QString("%1-%2").arg(number, text);
         files.append(newFileName);

         QFile::rename(QString("%1/%2").arg(mGitBase->getWorkingDir(), filename),
                       QString("%1/%2").arg(mGitBase->getWorkingDir(), newFileName));
         ++val;
      }
   }

   if (val != shaList.count())
      QLog_Error("Git", QString("Problem generating patches. Stop after {%1} iterations").arg(val));

   return qMakePair(true, files.join("<br>"));
}

bool GitPatches::applyPatch(const QString &fileName, bool asCommit)
{
   QLog_Debug("Git", QString("Applying patch: {%1} %2").arg(fileName, asCommit ? QString("as commit.") : QString()));

   auto cmd = asCommit ? QString("git am --signof ") : QString("git apply ");

   cmd.append(fileName);

   QLog_Trace("Git", QString("Applaying patch: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret.success;
}

GitExecResult GitPatches::stagePatch(const QString &fileName) const
{
   QLog_Debug("Git", QString("Staging patch: {%1}").arg(fileName));

   const auto cmd = QString("git apply --cached %1").arg(fileName);

   QLog_Trace("Git", QString("Staging patch: {%1}").arg(cmd));

   return mGitBase->run(cmd);
}
