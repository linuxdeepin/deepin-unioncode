#include <GitAsyncProcess.h>
#include <GitBase.h>
#include <GitCache.h>
#include <GitTags.h>
#include <QLogger.h>

using namespace QLogger;

namespace
{
bool validateSha(const QString &sha)
{
   static QRegExp hexMatcher("^[0-9A-F]{40}$", Qt::CaseInsensitive);

   return !sha.isEmpty() && hexMatcher.exactMatch(sha);
}
}

GitTags::GitTags(const QSharedPointer<GitBase> &gitBase)
   : mGitBase(gitBase)
{
}

GitTags::GitTags(const QSharedPointer<GitBase> &gitBase, const QSharedPointer<GitCache> &cache)
   : mGitBase(gitBase)
   , mCache(cache)
{
}

bool GitTags::getRemoteTags() const
{
   if (!mCache.get())
   {
      QLog_Fatal("Git", QString("Getting remote tages without cache."));
      assert(mCache.get());
   }

   QLog_Debug("Git", QString("Getting remote tags"));

   const auto cmd = QString("git ls-remote --tags");

   QLog_Trace("Git", QString("Getting remote tags: {%1}").arg(cmd));

   const auto p = new GitAsyncProcess(mGitBase->getWorkingDir());
   connect(p, &GitAsyncProcess::signalDataReady, this, &GitTags::onRemoteTagsRecieved);

   const auto ret = p->run(cmd);

   return ret.success;
}

GitExecResult GitTags::addTag(const QString &tagName, const QString &tagMessage, const QString &sha)
{
   QLog_Debug("Git", QString("Adding a tag: {%1}").arg(tagName));

   const auto cmd = QString("git tag -a %1 %2 -m \"%3\"").arg(tagName, sha, tagMessage);

   QLog_Trace("Git", QString("Adding a tag: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitTags::removeTag(const QString &tagName, bool remote)
{
   QLog_Debug("Git", QString("Removing tag: {%1}").arg(tagName));

   GitExecResult ret;

   if (remote)
   {
      const auto cmd = QString("git push origin --delete %1").arg(tagName);

      QLog_Trace("Git", QString("Removing tag: {%1}").arg(cmd));

      ret = mGitBase->run(cmd);
   }

   if (!remote || (remote && ret.success))
   {
      const auto cmd = QString("git tag -d %1").arg(tagName);

      QLog_Trace("Git", QString("Removing the tag locally: {%1}").arg(cmd));

      ret = mGitBase->run(cmd);
   }

   return ret;
}

GitExecResult GitTags::pushTag(const QString &tagName)
{
   QLog_Debug("Git", QString("Pushing a tag: {%1}").arg(tagName));

   const auto cmd = QString("git push origin %1").arg(tagName);

   QLog_Trace("Git", QString("Pushing a tag: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);

   return ret;
}

GitExecResult GitTags::getTagCommit(const QString &tagName)
{
   QLog_Debug("Git", QString("Getting the commit of a tag: {%1}").arg(tagName));

   const auto cmd = QString("git rev-list -n 1 %1").arg(tagName);

   QLog_Trace("Git", QString("Getting the commit of a tag: {%1}").arg(cmd));

   const auto ret = mGitBase->run(cmd);
   const auto output = ret.output.trimmed();

   return qMakePair(ret.success, output);
}

void GitTags::onRemoteTagsRecieved(GitExecResult result)
{
   QMap<QString, QString> tags;

   if (result.success)
   {
      const auto tagsTmp = result.output.split("\n");

      for (const auto &tag : tagsTmp)
      {
         if (tag != "\n" && !tag.isEmpty())
         {
            const auto isDereferenced = tag.contains("^{}");
            const auto sha = tag.split('\t').constFirst();
            const auto tagName = tag.split('\t').last().remove("refs/tags/").remove("^{}");

            if (validateSha(sha))
            {
               if (isDereferenced)
                  tags[tagName] = sha;
               else if (!tags.contains(tagName))
                  tags[tagName] = sha;
            }
         }
      }
   }

   mCache->updateTags(std::move(tags));
}
