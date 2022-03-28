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

#include <QFrame>

class GitCache;
class GitBase;
class QLabel;
class FileListWidget;
class CommitInfoPanel;

class CommitInfoWidget : public QFrame
{
   Q_OBJECT

signals:
   void signalOpenFileCommit(const QString &currentSha, const QString &previousSha, const QString &file, bool isCached);
   void signalOpenFileContextMenu(const QString &, int);
   void signalShowFileHistory(const QString &fileName);

   /**
    * @brief signalEditFile Signal triggered when the user wants to edit a file and is running GitQlient from QtCreator.
    * @param fileName The file name
    * @param line The line
    * @param column The column
    */
   void signalEditFile(const QString &fileName, int line, int column);

public:
   explicit CommitInfoWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                             QWidget *parent = nullptr);

   void configure(const QString &sha);
   QString getCurrentCommitSha() const;
   void clear();

private:
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;
   QString mCurrentSha;
   QString mParentSha;
   CommitInfoPanel *mInfoPanel = nullptr;
   FileListWidget *mFileListWidget = nullptr;
};
