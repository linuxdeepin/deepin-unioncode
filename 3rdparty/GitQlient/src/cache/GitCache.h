#pragma once

/****************************************************************************************
 ** GitQlient is an application to manage and operate one or several Git repositories. With
 ** GitQlient you will be able to add commits, branches and manage all the options Git provides.
 ** Copyright (C) 2021  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This program is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 2 of the License, or (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/

#include <CommitInfo.h>
#include <RevisionFiles.h>
#include <lanes.h>

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QSharedPointer>

#include <optional>

struct WipRevisionInfo;

class GitCache : public QObject
{
   Q_OBJECT

signals:
   void signalCacheUpdated();

public:
   struct LocalBranchDistances
   {
      int aheadOrigin = 0;
      int behindOrigin = 0;
   };

   explicit GitCache(QObject *parent = nullptr);
   ~GitCache();

   int commitCount() const;

   CommitInfo commitInfo(const QString &sha);
   CommitInfo commitInfo(int row);
   CommitInfo searchCommitInfo(const QString &text, int startingPoint = 0, bool reverse = false);
   bool isCommitInCurrentGeneologyTree(const QString &sha);
   bool updateWipCommit(const WipRevisionInfo &wipInfo);
   void insertCommit(CommitInfo commit);
   void updateCommit(const QString &oldSha, CommitInfo newCommit);

   bool insertRevisionFiles(const QString &sha1, const QString &sha2, const RevisionFiles &file);
   std::optional<RevisionFiles> revisionFile(const QString &sha1, const QString &sha2) const;

   void clearReferences();
   void insertReference(const QString &sha, References::Type type, const QString &reference);
   void deleteReference(const QString &sha, References::Type type, const QString &reference);
   bool hasReferences(const QString &sha);
   QStringList getReferences(const QString &sha, References::Type type);
   QString getShaOfReference(const QString &referenceName, References::Type type) const;
   void reloadCurrentBranchInfo(const QString &currentBranch, const QString &currentSha);

   void setUntrackedFilesList(QVector<QString> untrackedFiles);
   bool pendingLocalChanges();

   QVector<QPair<QString, QStringList>> getBranches(References::Type type);
   QMap<QString, QString> getTags(References::Type tagType) const;

   void updateTags(QMap<QString, QString> remoteTags);

   bool isInitialized() const { return mInitialized; }

private:
   friend class GitRepoLoader;

   bool mInitialized = false;
   bool mConfigured = true;
   Lanes mLanes;
   QVector<QString> mUntrackedFiles;

   mutable QMutex mCommitsMutex;
   QVector<CommitInfo *> mCommits;
   QHash<QString, CommitInfo> mCommitsMap;

   mutable QMutex mRevisionsMutex;
   QHash<QPair<QString, QString>, RevisionFiles> mRevisionFilesMap;

   mutable QMutex mReferencesMutex;
   QHash<QString, References> mReferences;

   void setup(const WipRevisionInfo &wipInfo, QVector<CommitInfo> commits);
   void setConfigurationDone() { mConfigured = true; }

   bool insertRevisionFile(const QString &sha1, const QString &sha2, const RevisionFiles &file);
   void insertWipRevision(const WipRevisionInfo &wipInfo);
   RevisionFiles fakeWorkDirRevFile(const QString &diffIndex, const QString &diffIndexCache);
   void calculateLanes(CommitInfo &c);
   auto searchCommit(const QString &text, int startingPoint = 0) const;
   auto reverseSearchCommit(const QString &text, int startingPoint = 0) const;
   void resetLanes(const CommitInfo &c, bool isFork);
   bool checkSha(const QString &originalSha, const QString &currentSha) const;
   void clearInternalData();
};
