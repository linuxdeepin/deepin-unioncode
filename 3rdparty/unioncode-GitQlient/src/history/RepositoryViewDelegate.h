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

#include <QStyledItemDelegate>
#include <QDateTime>

class CommitHistoryView;
class GitCache;
class GitBase;
class Lane;
class CommitInfo;
class GitServerCache;

namespace GitServer
{
struct PullRequest;
}

const int ROW_HEIGHT = 25;
const int LANE_WIDTH = 3 * ROW_HEIGHT / 4;

/**
 * @brief The RepositoryViewDelegate class is the delegate overloads the paint functionality in the RepositoryView. This
 * class is the responsible of painting the graph of the repository. In addition to paint all the columns, implements
 * special functionality to paint the tags, the branch names and how they are represented (local or remote)
 *
 */
class RepositoryViewDelegate : public QStyledItemDelegate
{
   Q_OBJECT

public:
   /**
    * @brief Default constructor.
    *
    * @param cache The cache for the current repository.
    * @param git The git object to execute git commands.
    * @param view The view that uses the delegate.
    */
   RepositoryViewDelegate(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                          const QSharedPointer<GitServerCache> &gitServerCache, CommitHistoryView *view);

   /**
    * @brief Overridden method to paint the different columns and rows in the view.
    *
    * @param p The painter device.
    * @param o The style options of the item.
    * @param i The index with the item data.
    */
   void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const override;
   /**
    * @brief The size hint returns the width and height for rendering purposes.
    *
    * @return QSize returns the size of a row.
    */
   QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;

protected:
   bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                    const QModelIndex &index) override;

private:
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitServerCache> mGitServerCache;
   CommitHistoryView *mView = nullptr;
   int diffTargetRow = -1;
   int mColumnPressed = -1;

   /**
    * @brief Paints the log column. This method is in charge of painting the commit message as well as tags or
    * branches.
    *
    * @param p The painter device.
    * @param o The style options of the item.
    * @param i The index with the item data.
    */
   void paintLog(QPainter *p, const QStyleOptionViewItem &o, const CommitInfo &commit, const QString &text) const;
   /**
    * @brief Method that sets up the configuration to paint the lane for the commit graph representation.
    *
    * @param p The painter device.
    * @param o The style options of the item.
    * @param index The index with the item data.
    */
   void paintGraph(QPainter *p, const QStyleOptionViewItem &o, const CommitInfo &commit) const;

   /**
    * @brief Specialization method called by @ref paintGrapth that does the actual lane painting.
    *
    * @param p The painter device.
    * @param type The type of lane to paint.
    * @param laneHeadPresent Tells the method if the lane contains a head.
    * @param x1 X coordinate where the painting starts
    * @param x2 X coordinate where the painting ends
    * @param col Color of the lane
    * @param activeCol Color of the active lane
    * @param mergeColor Color of the lane where the merge comes from in case the commit is a end-merge point.
    * @param isWip Tells the method if it's the WIP commit so it's painted differently.
    */
   void paintGraphLane(QPainter *p, const Lane &type, bool laneHeadPresent, int x1, int x2, const QColor &col,
                       const QColor &activeCol, const QColor &mergeColor, bool isWip = false,
                       bool hasChilds = true) const;

   /**
    * @brief Specialized method that paints a tag in the commit message column.
    *
    * @param painter The painter device.
    * @param opt The style options of the item.
    * @param startPoint The starting X coordinate for the tag.
    * @param sha The SHA reference to paint. It can be local branch, remote branch, tag or it could be detached.
    */
   void paintTagBranch(QPainter *painter, QStyleOptionViewItem opt, int &startPoint, const QString &sha) const;

   /**
    * @brief Specialized method that paints a tag in the commit message column.
    *
    * @param painter The painter device.
    * @param opt The style options of the item.
    * @param startPoint The starting X coordinate for the tag.
    * @param pr The PullRequest status.
    */
   void paintPrStatus(QPainter *painter, QStyleOptionViewItem opt, int &startPoint,
                      const GitServer::PullRequest &pr) const;

   /**
    * @brief getMergeColor Returns the color to be used for painting the external circle of the node. This methods
    * searches the origin of the merge and uses the same lane color.
    * @param currentLane The current lane type.
    * @param commit The current commit.
    * @param currentLaneIndex The current index of the lane.
    * @param defaultColor The default color in case it's not a merge.
    * @param isSet Boolean used as a shortcut. If the current iteration is a merge it will change the value for the
    * following lanes.
    * @return Returns the color of the lane that merges into the current node, otherwise it returns @p defaultColor.
    */
   QColor getMergeColor(const Lane &currentLane, const CommitInfo &commit, int currentLaneIndex,
                        const QColor &defaultColor, bool &isSet) const;
};
