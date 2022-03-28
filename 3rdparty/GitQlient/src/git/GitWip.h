#pragma once

#include <QSharedPointer>

#include <WipRevisionInfo.h>

class GitBase;
class GitCache;

class GitWip
{
public:
   explicit GitWip(const QSharedPointer<GitBase> &git, const QSharedPointer<GitCache> &cache);

   QVector<QString> getUntrackedFiles() const;
   bool updateWip() const;
   WipRevisionInfo getWipInfo() const;

private:
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitCache> mCache;
};
