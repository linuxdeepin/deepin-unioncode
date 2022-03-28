#pragma once

#include <QByteArray>
#include <QStringList>
#include <QVector>

class RevisionFiles
{
public:
   enum StatusFlag
   {
      MODIFIED = 1,
      DELETED = 2,
      NEW = 4,
      RENAMED = 8,
      COPIED = 16,
      UNKNOWN = 32,
      IN_INDEX = 64,
      CONFLICT = 128,
      PARTIALLY_CACHED = 256
   };

   RevisionFiles() = default;
   ~RevisionFiles();
   RevisionFiles(const QString &diff, bool cached = false);

   bool isValid() const;
   bool operator==(const RevisionFiles &revFiles) const;
   bool operator!=(const RevisionFiles &revFiles) const;

   QVector<int> mergeParent;
   QVector<QString> mFiles;

   // helper functions
   int count() const { return mFiles.count(); }
   bool statusCmp(int idx, StatusFlag sf) const;
   const QString extendedStatus(int idx) const;
   void setStatus(const QString &rowSt, bool isStaged = false);
   void setStatus(RevisionFiles::StatusFlag flag);
   void setStatus(int pos, RevisionFiles::StatusFlag flag);
   void appendStatus(int pos, RevisionFiles::StatusFlag flag);
   int getStatus(int pos) const { return mFileStatus.at(pos); }
   void setOnlyModified(bool onlyModified) { mOnlyModified = onlyModified; }
   int getFilesCount() const { return mFileStatus.size(); }
   void appendExtStatus(const QString &file) { mRenamedFiles.append(file); }
   QString getFile(int index) const { return mFiles.at(index); }
   QStringList getFiles() const { return mFiles.toList(); }
   bool containsFile(const QString &fileName) { return mFiles.contains(fileName); }

private:
   // Status information is split in a flags vector and in a string
   // vector in 'status' are stored flags according to the info returned
   // by 'git diff-tree' without -C option.
   // In case of a working directory file an IN_INDEX flag is or-ed together in
   // case file is present in git index.
   // If file is renamed or copied an entry in 'extStatus' stores the
   // value returned by 'git diff-tree -C' plus source and destination
   // files info.
   // When status of all the files is 'modified' then onlyModified is
   // set, this let us to do some optimization in this common case
   bool mOnlyModified = true;
   QVector<int> mFileStatus;
   QVector<QString> mRenamedFiles;

   void setExtStatus(const QString &rowSt, int parNum);
};
