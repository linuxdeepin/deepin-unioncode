#include "CommitInfo.h"

#include <QStringList>

const QString CommitInfo::ZERO_SHA = QString("0000000000000000000000000000000000000000");
const QString CommitInfo::INIT_SHA = QString("4b825dc642cb6eb9a060e54bf8d69288fbee4904");

CommitInfo::CommitInfo(QByteArray commitData, const QString &gpg, bool goodSignature)
   : gpgKey(gpg)
   , mGoodSignature(goodSignature)
{
   parseDiff(commitData, 0);
}

CommitInfo::CommitInfo(QByteArray data)
{
    parseDiff(data, 1);
}

void CommitInfo::parseDiff(QByteArray &data, int startingField)
{
    if (const auto fields = QString::fromUtf8(data).split('\n'); fields.count() > 0)
    {
       const auto firstField = fields.constFirst();
       auto combinedShas = fields.at(startingField++);
       auto shas = combinedShas.split('X');
       sha = shas.takeFirst().remove(0, 1);

       if (!shas.isEmpty())
       {
 #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
          mParentsSha = shas.takeFirst().split(' ', Qt::SkipEmptyParts);
 #else
          mParentsSha = shas.takeFirst().split(' ', QString::SkipEmptyParts);
 #endif
       }
       committer = fields.at(startingField++);
       author = fields.at(startingField++);
       dateSinceEpoch = std::chrono::seconds(fields.at(startingField++).toInt());
       shortLog = fields.at(startingField);

       for (auto i = 6; i < fields.count(); ++i)
          longLog += fields.at(i) + '\n';

       longLog = longLog.trimmed();
    }
}

CommitInfo::CommitInfo(const QString &sha, const QStringList &parents, std::chrono::seconds commitDate,
                       const QString &log)
   : sha(sha)
   , dateSinceEpoch(commitDate)
   , shortLog(log)
   , mParentsSha(parents)
{
}

bool CommitInfo::operator==(const CommitInfo &commit) const
{
   return sha.startsWith(commit.sha) && mParentsSha == commit.mParentsSha && committer == commit.committer
       && author == commit.author && dateSinceEpoch == commit.dateSinceEpoch && shortLog == commit.shortLog
       && longLog == commit.longLog && mLanes == commit.mLanes;
}

bool CommitInfo::operator!=(const CommitInfo &commit) const
{
   return !(*this == commit);
}

bool CommitInfo::contains(const QString &value)
{
   return sha.startsWith(value, Qt::CaseInsensitive) || shortLog.contains(value, Qt::CaseInsensitive)
       || committer.contains(value, Qt::CaseInsensitive) || author.contains(value, Qt::CaseInsensitive);
}

int CommitInfo::parentsCount() const
{
   auto count = mParentsSha.count();

   if (count > 0 && mParentsSha.contains(CommitInfo::INIT_SHA))
      --count;

   return count;
}

QString CommitInfo::firstParent() const
{
   return !mParentsSha.isEmpty() ? mParentsSha.at(0) : QString();
}

QStringList CommitInfo::parents() const
{
   return mParentsSha;
}

bool CommitInfo::isInWorkingBranch() const
{
   for (const auto &child : mChilds)
   {
      if (child->sha == CommitInfo::ZERO_SHA)
      {
         return true;
         break;
      }
   }

   return false;
}

void CommitInfo::setLanes(QVector<Lane> lanes)
{
   this->mLanes.clear();
   this->mLanes.squeeze();
   this->mLanes = std::move(lanes);
}

bool CommitInfo::isValid() const
{
   static QRegExp hexMatcher("^[0-9A-F]{40}$", Qt::CaseInsensitive);

   return !sha.isEmpty() && hexMatcher.exactMatch(sha);
}

int CommitInfo::getActiveLane() const
{
   auto i = 0;

   for (auto lane : mLanes)
   {
      if (lane.isActive())
         return i;
      else
         ++i;
   }

   return -1;
}

void CommitInfo::removeChild(CommitInfo *commit)
{
   if (mChilds.contains(commit))
      mChilds.removeAll(commit);
}

QString CommitInfo::getFirstChildSha() const
{
   if (!mChilds.isEmpty())
      mChilds.constFirst();
   return QString();
}
