#include "RevisionFiles.h"

RevisionFiles::RevisionFiles(const QString &diff, bool cached)
{
   auto parNum = 1;

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
   const auto lines = diff.split("\n", Qt::SkipEmptyParts);
#else
   const auto lines = diff.split("\n", QString::SkipEmptyParts);
#endif

   for (auto line : lines)
   {
      if (line[0] == ':') // avoid sha's in merges output
      {
         if (line[1] == ':')
         {
            mFiles.append(line.section('\t', -1));
            setStatus("M");
            mergeParent.append(parNum);
         }
         else
         {
            if (line.at(98) == '\t') // Faster parsing in normal case
            {
               auto fields = line.split(" ");
               const auto dstSha = fields.at(3);
               auto fileIsCached = !dstSha.startsWith(QStringLiteral("000000"));
               const auto flag = fields.at(4).at(0);

               mFiles.append(line.mid(99));
               setStatus(flag, cached ? cached : fileIsCached);
               mergeParent.append(parNum);
            }
            else // It's a rename or a copy, we are not in fast path now!
               setExtStatus(line.mid(97), parNum);
         }
      }
      else
         ++parNum;
   }
}

RevisionFiles::~RevisionFiles()
{
   mFileStatus.clear();
   mFileStatus.squeeze();
   mRenamedFiles.clear();
   mRenamedFiles.squeeze();
   mergeParent.clear();
   mergeParent.squeeze();
   mFiles.clear();
   mFiles.squeeze();
}

bool RevisionFiles::isValid() const
{
   return !(mFiles.empty() && mFileStatus.empty() && mRenamedFiles.empty());
}

bool RevisionFiles::operator==(const RevisionFiles &revFiles) const
{
   return mFiles == revFiles.mFiles && mOnlyModified == revFiles.mOnlyModified && mergeParent == revFiles.mergeParent
       && mFileStatus == revFiles.mFileStatus && mRenamedFiles == revFiles.mRenamedFiles;
}

bool RevisionFiles::operator!=(const RevisionFiles &revFiles) const
{
   return !(*this == revFiles);
}

bool RevisionFiles::statusCmp(int idx, RevisionFiles::StatusFlag sf) const
{
   if (idx >= mFileStatus.count())
      return false;

   return (mOnlyModified ? MODIFIED : mFileStatus.at(static_cast<int>(idx))) & sf;
}

const QString RevisionFiles::extendedStatus(int idx) const
{
   /*
         rf.extStatus has size equal to position of latest copied/renamed file,
         that could be lower then count(), so we have to explicitly check for
         an out of bound condition.
      */
   return !mRenamedFiles.isEmpty() && idx < mRenamedFiles.count() ? mRenamedFiles.at(idx) : "";
}

void RevisionFiles::setStatus(const QString &rowSt, bool isStaged)
{
   switch (rowSt.at(0).toLatin1())
   {
      case 'M':
      case 'T':
         mFileStatus.append(RevisionFiles::MODIFIED);
         if (isStaged)
            mFileStatus[mFileStatus.count() - 1] |= RevisionFiles::IN_INDEX;
         break;
      case 'U':
         mFileStatus.append(RevisionFiles::MODIFIED);
         mFileStatus[mFileStatus.count() - 1] |= RevisionFiles::CONFLICT;
         if (isStaged)
            mFileStatus[mFileStatus.count() - 1] |= RevisionFiles::IN_INDEX;
         mOnlyModified = false;
         break;
      case 'D':
         mFileStatus.append(RevisionFiles::DELETED);
         mOnlyModified = false;
         if (isStaged)
            mFileStatus[mFileStatus.count() - 1] |= RevisionFiles::IN_INDEX;
         break;
      case 'A':
         mFileStatus.append(RevisionFiles::NEW);
         mOnlyModified = false;
         if (isStaged)
            mFileStatus[mFileStatus.count() - 1] |= RevisionFiles::IN_INDEX;
         break;
      case '?':
         mFileStatus.append(RevisionFiles::UNKNOWN);
         mOnlyModified = false;
         break;
      default:
         mFileStatus.append(RevisionFiles::MODIFIED);
         break;
   }
}

void RevisionFiles::setStatus(RevisionFiles::StatusFlag flag)
{
   mFileStatus.append(flag);

   if (flag == RevisionFiles::DELETED || flag == RevisionFiles::NEW || flag == RevisionFiles::UNKNOWN)
      mOnlyModified = false;
}

void RevisionFiles::setStatus(int pos, RevisionFiles::StatusFlag flag)
{
   mFileStatus[pos] = flag;
}

void RevisionFiles::appendStatus(int pos, RevisionFiles::StatusFlag flag)
{
   mFileStatus[pos] |= flag;
}

void RevisionFiles::setExtStatus(const QString &rowSt, int parNum)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
   const QStringList sl(rowSt.split('\t', Qt::SkipEmptyParts));
#else
   const QStringList sl(rowSt.split('\t', QString::SkipEmptyParts));
#endif
   if (sl.count() != 3)
      return;

   // we want store extra info with format "orig --> dest (Rxx%)"
   // but git give us something like "Rxx\t<orig>\t<dest>"
   QString type = sl[0];
   type.remove(0, 1);
   const QString &orig = sl[1];
   const QString &dest = sl[2];
   const QString extStatusInfo(orig + " --> " + dest + " (" + QString::number(type.toInt()) + "%)");

   mFiles.append(dest);
   mergeParent.append(parNum);
   setStatus(RevisionFiles::NEW);
   appendExtStatus(extStatusInfo);

   // simulate deleted orig file only in case of rename
   if (type.at(0) == 'R')
   {
      mFiles.append(orig);
      mergeParent.append(parNum);
      setStatus(RevisionFiles::DELETED);
      appendExtStatus(extStatusInfo);
   }

   setOnlyModified(false);
}
