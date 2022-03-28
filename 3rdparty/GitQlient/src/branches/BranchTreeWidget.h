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

#include <QTreeWidget>

class GitBase;
class GitCache;

class RefTreeWidget : public QTreeWidget
{
   Q_OBJECT

public:
   /**
    * @brief Default constructor
    * @param cache The GitQlient cache.
    * @param git The git object to perform Git operations.
    * @param parentThe parent widget if needed.
    */
   explicit RefTreeWidget(QWidget *parent = nullptr);
   /**
    * @brief focusOnBranch Sets the focus of the three in the item specified in  @p branch starting from the position @p
    * lastPos.
    * @param item The text to seach in the tree and set the focus.
    * @param lastPos Starting position for the search.
    * @return
    */
   int focusOnBranch(const QString &itemText, int startSearchPos = -1);

protected:
   QVector<QTreeWidgetItem *> findChildItem(const QString &text) const;
};

/*!
 \brief The BranchTreeWidget class shows all the information regarding the branches and its position respect master and
 its remote branch.

*/
class BranchTreeWidget : public RefTreeWidget
{
   Q_OBJECT

signals:
   void fullReload();
   void logReload();

   /*!
    \brief Signal triggered when the user selects a commit via branch or tag selection.

    \param sha The selected sha.
   */
   void signalSelectCommit(const QString &sha);
   /*!
    \brief Signal triggered when a merge is required.

    \param currentBranch The current branch.
    \param fromBranch The branch to merge into the current one.
   */
   void signalMergeRequired(const QString &currentBranch, const QString &fromBranch);
   /*!
    * \brief signalPullConflict Signal triggered when trying to pull and a conflict happens.
    */
   void signalPullConflict();

   /**
    * @brief signalFetchPerformed Signal triggered when a deep fetch is performed.
    */
   void signalFetchPerformed();
   /**
    * @brief signalRefreshPRsCache Signal that refreshes PRs cache.
    */
   void signalRefreshPRsCache();

   /**
    * @brief Signal triggered when a merge with squash behavior has been requested. Since it involves a lot of changes
    * at UI level this action is not performed here.
    *
    * @param origin The branch to merge from.
    * @param destination The branch to merge into.
    */
   void mergeSqushRequested(const QString &origin, const QString &destination);

public:
   /*!
    \brief Default constructor.

    \param git The git object to perform Git operations.
    \param parent The parent widget if needed.
   */
   explicit BranchTreeWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                             QWidget *parent = nullptr);
   /*!
    \brief Configures the widget to be the local branches widget.

    \param isLocal True if the current widget shows local branches, otherwise false.
   */
   void setLocalRepo(const bool isLocal) { mLocal = isLocal; }

   /**
    * @brief reloadCurrentBranchLink Reloads the link to the current branch.
    */
   void reloadCurrentBranchLink() const;

private:
   bool mLocal = false;
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;

   /*!
    \brief Shows the context menu.

    \param pos The position of the menu.
   */
   void showBranchesContextMenu(const QPoint &pos);
   /*!
    \brief Checks out the branch selected by the \p item.

    \param item The item that contains the data of the branch.
   */
   void checkoutBranch(QTreeWidgetItem *item);
   /*!
    \brief Selects the commit of the given \p item branch.

    \param item The item that contains the data of the branch selected to extract the commit SHA.
   */
   void selectCommit(QTreeWidgetItem *item);

   /**
    * @brief onSelectionChanged Process when a selection has changed.
    */
   void onSelectionChanged();
};
