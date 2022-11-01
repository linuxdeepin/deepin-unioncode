#include "GitWip.h"

#include <GitBase.h>
#include <GitCache.h>

#include <QLogger.h>

#include <QFile>

using namespace QLogger;

GitWip::GitWip(const QSharedPointer<GitBase> &git, const QSharedPointer<GitCache> &cache)
   : mGit(git)
   , mCache(cache)
{
}

QVector<QString> GitWip::getUntrackedFiles() const
{
   QLog_Debug("Git", QString("Executing getUntrackedFiles."));

   auto runCmd = QString("git ls-files --others");
   const auto exFile = QString("info/exclude");
   const auto path = QString("%1/%2").arg(mGit->getGitDir(), exFile);

   if (QFile::exists(path))
      runCmd.append(QString(" --exclude-from=$%1$").arg(path));

   runCmd.append(QString(" --exclude-per-directory=$%1$").arg(".gitignore"));

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
   const auto ret = mGit->run(runCmd).output.split('\n', Qt::SkipEmptyParts).toVector();
#else
   const auto ret = mGit->run(runCmd).output.split('\n', QString::SkipEmptyParts).toVector();
#endif

   return ret;
}

WipRevisionInfo GitWip::getWipInfo() const
{
   QLog_Debug("Git", QString("Executing processWip."));

   const auto ret = mGit->run("git rev-parse --revs-only HEAD");

   if (ret.success)
   {
      QString diffIndex;
      QString diffIndexCached;

      auto parentSha = ret.output.trimmed();

      if (parentSha.isEmpty())
         parentSha = CommitInfo::INIT_SHA;

      const auto ret3 = mGit->run(QString("git diff-index %1").arg(parentSha));
      diffIndex = ret3.success ? ret3.output : QString();

      const auto ret4 = mGit->run(QString("git diff-index --cached %1").arg(parentSha));
      diffIndexCached = ret4.success ? ret4.output : QString();

      return { parentSha, diffIndex, diffIndexCached };
   }

   return {};
}

bool GitWip::updateWip() const
{
   const auto files = getUntrackedFiles();
   mCache->setUntrackedFilesList(std::move(files));

   if (const auto wipInfo = getWipInfo(); wipInfo.isValid())
      return mCache->updateWipCommit(wipInfo);

   return false;
}
