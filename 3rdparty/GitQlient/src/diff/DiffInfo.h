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

#include <QVector>
#include <QUuid>
#include <QStringList>

struct ChunkDiffInfo
{
   struct ChunkInfo
   {
#if QT_VERSION <= QT_VERSION_CHECK(5, 13, 0)
      ChunkInfo() = default;
#endif
      ChunkInfo(const QString &_id)
         : id(_id)
      {
      }
      int startLine = -1;
      int endLine = -1;
      bool addition = false;
      QString id;

      bool isValid() const { return startLine != -1 && endLine != -1; }
   };

   ChunkDiffInfo()
      : newFile(ChunkInfo(id))
      , oldFile(ChunkInfo(id))
   {
   }
   ChunkDiffInfo(bool baseOld, const ChunkInfo &_newFile, const ChunkInfo &_oldFile)
      : baseIsOldFile(baseOld)
      , newFile(_newFile)
      , oldFile(_oldFile)
   {
      newFile.id = id;
      oldFile.id = id;
   }
   bool operator==(const ChunkDiffInfo &info) const { return id == info.id; }
   bool isValid() const { return newFile.isValid() || oldFile.isValid(); }

   QString id = QUuid::createUuid().toString();
   bool baseIsOldFile = true;
   ChunkInfo newFile;
   ChunkInfo oldFile;
};

struct DiffInfo
{
   QStringList fullDiff;
   QStringList newFileDiff;
   QStringList oldFileDiff;
   QVector<ChunkDiffInfo> chunks;
};
