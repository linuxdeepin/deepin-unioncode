#include "GitSubmodules.h"

#include <GitBase.h>
#include <QLogger.h>

#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>
#include <QVector>

using namespace QLogger;

GitSubmodules::GitSubmodules(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

QVector<QString> GitSubmodules::getSubmodules()
{
   QLog_Debug("Git", QString("Getting submodules"));

   const auto cmd = QString("git config --file .gitmodules --name-only --get-regexp path");

   QLog_Trace("Git", QString("Getting submodules: {%1}").arg(cmd));

   QVector<QString> submodulesList;

   if (const auto ret = mGitBase->run(cmd); ret.success)
   {
      const auto submodules = ret.output.split('\n');

      for (const auto &submodule : submodules)
      {
         if (!submodule.isEmpty() && submodule != "\n")
            submodulesList.append(submodule.split('.').at(1));
      }
   }

   return submodulesList;
}

bool GitSubmodules::submoduleAdd(const QString &url, const QString &name)
{
   QLog_Debug("Git", QString("Adding a submodule: {%1} {%2}").arg(url, name));

   const auto cmd = QString("git submodule add %1 %2").arg(url, name);

   QLog_Trace("Git", QString("Adding a submodule: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd).success;

   return ret;
}

bool GitSubmodules::submoduleUpdate(const QString &submodule)
{
   if (submodule.isEmpty())
      QLog_Debug("Git", QString("Updating all submodules"));
   else
      QLog_Debug("Git", QString("Updating submodule: {%1}").arg(submodule));

   auto cmd = QString("git submodule update --init --recursive");

   if (!submodule.isEmpty())
      cmd.append(QString(" %1").arg(submodule));

   QLog_Trace("Git", QString("Updating submodules: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd).success;

   return ret;
}

bool GitSubmodules::submoduleRemove(const QString &submodule)
{
   QLog_Debug("Git", QString("Removing a submodule: {%1}").arg(submodule));

   auto cmd = QString("git submodule deinit -f %1").arg(submodule);

   QLog_Trace("Git", QString("Deinitializing the submodule: {%1}").arg(cmd));

   auto ret = mGitBase->run(cmd);

   cmd = QString("git rm -f --cached %1").arg(submodule);

   QLog_Trace("Git", QString("Removing cache: {%1}").arg(cmd));

   ret = mGitBase->run(cmd);

   cmd = QString("rm -rf %1/.git/modules/%2").arg(mGitBase->getGitDir(), submodule);

   QLog_Trace("Git", QString("Removing the submodule: {%1}").arg(cmd));

   ret = mGitBase->run(cmd);

   QFile gitmodules(QString("%1/.gitmodules").arg(mGitBase->getWorkingDir()));
   QTemporaryFile gitTmp;

   if (gitmodules.open(QIODevice::ReadOnly) && gitTmp.open())
   {
      QTextStream out(&gitmodules);
      QTextStream in(&gitTmp);
      auto removed = false;

      while (!out.atEnd())
      {
         auto line = out.readLine();

         if (line.contains(QString("[submodule \"%1\"]").arg(submodule)))
         {
            out.readLine();
            out.readLine();
            removed = true;
         }
         else
            in << line.append('\n');
      }
      gitmodules.close();
      gitTmp.close();

      if (removed)
      {
         gitmodules.remove();
         gitTmp.copy(QString("%1/.gitmodules").arg(mGitBase->getWorkingDir()));
      }
   }

   return ret.success;
}
