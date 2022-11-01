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

#include <QAbstractItemModel>
#include <QSharedPointer>

class GitCache;
class GitBase;
class CommitInfo;
class GitServerCache;
enum class CommitHistoryColumns;

/**
 * @brief The CommitHistoryModel contains the data model (is the Model in the MVC pattern) that will be displayed by the
 * view.
 *
 * @class CommitHistoryModel CommitHistoryModel.h "CommitHistoryModel.h"
 */
class CommitHistoryModel : public QAbstractItemModel
{
   Q_OBJECT
public:
   /**
    * @brief The default constructor.
    *
    * @param cache The internal cache of the current repository.
    * @param git The git object to execute Git operations.
    * @param parent The parent widget if needed.
    */
   explicit CommitHistoryModel(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                               const QSharedPointer<GitServerCache> &gitServerCache, QObject *parent = nullptr);

   /**
    * @brief Clears the contents without deleting the cache.
    */
   void clear();
   /**
    * @brief Method used to retrieve the sha of a specific row. The view access this method instead of the internal
    * cache because the row could change when a proxy filter is used between the View and the Model data.
    *
    * @param row The row to get the SHA from.
    * @return QString The SHA.
    */
   QString sha(int row) const;

   /**
    * @brief Returns the data stored under the given \p role for the item referred to by the \p index
    *
    * @param index The index to get the data from.
    * @param role The role from where to extract the data.
    * @return QVariant The data value.
    */
   QVariant data(const QModelIndex &index, int role) const override;
   /**
    * @brief Returns the header data for a specific column.
    *
    * @param s The column.
    * @param o The orientation. Refers to what header to extract the data: Horizontal or Vertical.
    * @param role The role from where to extract the data.
    * @return QVariant The data.
    */
   QVariant headerData(int s, Qt::Orientation o, int role = Qt::DisplayRole) const override;
   /**
    * @brief Returns the index of the item in the model specified by the given row, column and parent index
    *
    * @param r The row number.
    * @param c The column number.
    * @param par The parent index.
    * @return QModelIndex The index.
    */
   QModelIndex index(int r, int c, const QModelIndex &par = QModelIndex()) const override;
   /**
    * @brief Returns the parent of the model item with the given index. If the item has no parent, an invalid
    * QModelIndex is returned.
    *
    * @param index The index.
    * @return QModelIndex The parent of the given \p index.
    */
   QModelIndex parent(const QModelIndex &index) const override;
   /**
    * @brief Returns the number of rows of an index.
    *
    * @param par The index to retrieve the rows.
    * @return int The number of rows.
    */
   int rowCount(const QModelIndex &par = QModelIndex()) const override;
   /**
    * @brief Returns if an index contains children.
    *
    * @param par The index.
    * @return bool True if has children, otherwise false.
    */
   bool hasChildren(const QModelIndex &par = QModelIndex()) const override;
   /**
    * @brief Returns the number of columns of the model.
    *
    * @return int The number of columns.
    */
   int columnCount(const QModelIndex &) const override { return mColumns.count(); }
   /**
    * @brief Resets the model when new revisions are available.
    *
    * @param totalCommits The total of new revisions.
    */
   void onNewRevisions(int totalCommits);
   /*!
    * \brief Gets the number of columns in the model.
    * \return The number of columns.
    */
   int columnCount() const { return mColumns.count(); }

private:
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitServerCache> mGitServerCache;
   QMap<CommitHistoryColumns, QString> mColumns;

   /**
    * @brief Returns the tool tip data.
    *
    * @param r The commit to generate the tooltip data.
    * @return QVariant The tool tip data.
    */
   QVariant getToolTipData(const CommitInfo &r) const;
   /**
    * @brief Returns the data that will be display for every \p column.
    *
    * @param rev The commit info to retrieve the data that will be displayed.
    * @param column The column where the data will be shown.
    * @return QVariant The data to be shown.
    */
   QVariant getDisplayData(const CommitInfo &rev, int column) const;
};
