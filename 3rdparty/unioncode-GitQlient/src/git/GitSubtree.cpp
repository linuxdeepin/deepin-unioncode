#include <GitBase.h>
#include <GitQlientSettings.h>
#include <GitSubtree.h>

#include <QLogger.h>

using namespace QLogger;

GitSubtree::GitSubtree(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

GitExecResult GitSubtree::add(const QString &url, const QString &ref, const QString &name, bool squash)
{
   QLog_Debug("UI", "Adding a subtree");

   GitQlientSettings settings(mGitBase->getGitDir());

   for (auto i = 0;; ++i)
   {
      const auto repo = settings.localValue(QString("Subtrees/%1.prefix").arg(i)).toString();

      if (repo == name)
      {
         settings.setLocalValue(QString("Subtrees/%1.url").arg(i), url);
         settings.setLocalValue(QString("Subtrees/%1.ref").arg(i), ref);

         auto cmd = QString("git subtree add --prefix=%1 %2 %3").arg(name, url, ref);

         QLog_Trace("Git", QString("Adding a subtree: {%1}").arg(cmd));

         if (squash)
            cmd.append(" --squash");

         auto ret = mGitBase->run(cmd);

         if (ret.output.contains("Cannot"))
            ret.success = false;

         return ret;
      }
      else if (repo.isEmpty())
      {
         settings.setLocalValue(QString("Subtrees/%1.prefix").arg(i), name);
         settings.setLocalValue(QString("Subtrees/%1.url").arg(i), url);
         settings.setLocalValue(QString("Subtrees/%1.ref").arg(i), ref);

         QLog_Trace("Git", QString("Updating subtree info: {%1}").arg(name));

         return { true, "" };
      }
   }

   return { false, "" };
}

GitExecResult GitSubtree::pull(const QString &url, const QString &ref, const QString &prefix) const
{
   QLog_Debug("UI", "Pulling a subtree");

   const auto cmd = QString("git subtree pull --prefix=%1 %2 %3").arg(prefix, url, ref);

   QLog_Trace("Git", QString("Pulling a subtree: {%1}").arg(cmd));

   auto ret = mGitBase->run(cmd);

   if (ret.output.contains("Cannot"))
      ret.success = false;

   return ret;
}

GitExecResult GitSubtree::push(const QString &url, const QString &ref, const QString &prefix) const
{
   QLog_Debug("UI", "Pushing changes to a subtree");

   const auto cmd = QString("git subtree push --prefix=%1 %2 %3").arg(prefix, url, ref);

   QLog_Trace("Git", QString("Pushing changes to a subtree: {%1}").arg(cmd));

   auto ret = mGitBase->run(cmd);

   if (ret.output.contains("Cannot"))
      ret.success = false;

   return ret;
}

GitExecResult GitSubtree::merge(const QString &sha) const
{
   QLog_Debug("UI", "Merging changes from the remote of a subtree");

   const auto cmd = QString("git subtree merge %1").arg(sha);

   QLog_Trace("Git", QString("Merging changes from the remote of a subtree: {%1}").arg(cmd));

   auto ret = mGitBase->run(cmd);

   if (ret.output.contains("Cannot"))
      ret.success = false;

   return ret;
}

GitExecResult GitSubtree::list() const
{
   QLog_Debug("UI", "Listing all subtrees");

   const auto cmd = QString("git log --pretty=format:%b --grep=git-subtree-dir");

   QLog_Trace("Git", QString("Listing all subtrees: {%1}").arg(cmd));

   return mGitBase->run(cmd);
}
