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

class BranchTreeWidget;
class QListWidget;
class QListWidgetItem;
class QLabel;
class GitBase;
class GitTags;
class GitCache;
class QPushButton;
class BranchesWidgetMinimal;
class BranchesViewDelegate;
class QTreeWidget;
class QTreeWidgetItem;
class RefTreeWidget;

/*!
 \brief BranchesWidget is the widget that creates the layout that contains all the widgets related with the display of
 branch information, such as BranchTreeWidget but also the widgets that show information for the tags, submodules and
 stashes.

*/
class BranchesWidget : public QFrame
{
   Q_OBJECT

signals:
   void fullReload();
   void logReload();

   /**
    * @brief panelsVisibilityChanged Signal triggered whenever the visibility of the panels in the BranchesWidget
    * changes.
    */
   void panelsVisibilityChanged();
   /*!
    \brief Signal triggered when the user selects a commit via branch or tag selection.

    \param sha The selected sha.
   */
   void signalSelectCommit(const QString &sha);
   /*!
    \brief Signal triggered when a the user wants to open a submodule as a new repository view. This signal will lead
    GitQlient to open a new tab in the general UI view and change the current repository view to that one.

    \param submoduleName The module name.
   */
   void signalOpenSubmodule(const QString &submoduleName);
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
    * @brief minimalViewStateChanged Signal triggered when the minimal view is active.
    */
   void minimalViewStateChanged(bool isActive);

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

    \param git The git object to perform Git commands.
    \param parent The parent widget if needed.
   */
   explicit BranchesWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                           QWidget *parent = nullptr);

   /**
    * @brief Destructor;
    */
   ~BranchesWidget();

   /**
    * @brief isMinimalViewActive Checks if the minimal view is active.
    * @return True if active, otherwise false.
    */
   bool isMinimalViewActive() const;

   /*!
    \brief This method configures the widget gathering all the information regarding branches, tags, stashes and
    submodules.

   */
   void showBranches();

   /**
    * @brief refreshCurrentBranchLink Reloads the current branch link
    */
   void refreshCurrentBranchLink();
   /*!
    \brief Clears all widget's information.

   */
   void clear();

   /**
    * @brief returnToSavedView Returns to the view mode previously saved. This methods bypasses the forceMinimalView.
    * method.
    */
   void returnToSavedView();

   /**
    * @brief forceMinimalView Forces the minimal view but temporarily: id doesn't save the state.
    */
   void forceMinimalView();

   /**
    * @brief onPanelsVisibilityChaned Reloads the visibility of the stash, submodules, and subtree panels.
    */
   void onPanelsVisibilityChaned();

private:
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitTags> mGitTags;
   BranchTreeWidget *mLocalBranchesTree = nullptr;
   BranchesViewDelegate *mLocalDelegate = nullptr;
   BranchTreeWidget *mRemoteBranchesTree = nullptr;
   BranchesViewDelegate *mRemotesDelegate = nullptr;
   BranchesViewDelegate *mTagsDelegate = nullptr;
   RefTreeWidget *mTagsTree = nullptr;
   QListWidget *mStashesList = nullptr;
   QLabel *mStashesCount = nullptr;
   QLabel *mStashesArrow = nullptr;
   QLabel *mSubmodulesCount = nullptr;
   QLabel *mSubmodulesArrow = nullptr;
   QListWidget *mSubmodulesList = nullptr;
   QLabel *mSubtreeCount = nullptr;
   QLabel *mSubtreeArrow = nullptr;
   QListWidget *mSubtreeList = nullptr;
   QPushButton *mMinimize = nullptr;
   QFrame *mFullBranchFrame = nullptr;
   BranchesWidgetMinimal *mMinimal = nullptr;
   QString mLastSearch;
   int mLastIndex;
   RefTreeWidget *mLastTreeSearched = nullptr;

   /**
    * @brief fullView Shows the full branches view.
    */
   void fullView();

   /**
    * @brief minimalView Shows the minimalistic branches view.
    */
   void minimalView();

   /*!
    \brief Method that for a given \p branch process all the information and creates the item that will be stored in the
    local branches BranchTreeWidget.

    \param branch The branch to be added in the tree widget.
   */
   void processLocalBranch(const QString &sha, QString branch);
   /*!
    \brief Method that for a given \p branch process all the information and creates the item that will be stored in the
    remote branches BranchTreeWidget.

    \param branch The remote branch to be added in the tree widget.
   */
   void processRemoteBranch(const QString &sha, QString branch);
   /*!
    \brief Process all the tags and adds them into the QListWidget.

   */
   void processTags();
   /*!
    \brief Process all the stashes and adds them into the QListWidget.

   */
   void processStashes();
   /*!
    \brief Process all the submodules and adds them into QListWidget.

   */
   void processSubmodules();

   /**
    * @brief processSubtrees Process all the subtrees and adds them into the QListWidget.
    */
   void processSubtrees();
   /*!
    \brief Once all the items have been added to the conrresponding BranchTreeWidget, the columns are adjusted to show
    the data correctly from a UI point of view.

    \param treeWidget
   */
   void adjustBranchesTree(BranchTreeWidget *treeWidget);
   /*!
    \brief Shows the tags context menu.

    \param p The position where the menu will be displayed.
   */
   void showTagsContextMenu(const QPoint &p);
   /*!
    \brief Shows the stashes context menu.

    \param p The position where the menu will be displayed.
   */
   void showStashesContextMenu(const QPoint &p);
   /*!
    \brief Shows the submodules context menu.

    \param p The position where the menu will be displayed.
   */
   void showSubmodulesContextMenu(const QPoint &p);

   /**
    * @brief showSubtreesContextMenu Shows the subtrees context menu.
    * @param p The position where the menu will be displayed.
    */
   void showSubtreesContextMenu(const QPoint &p);
   /*!
    \brief Expands or contracts the stashes list widget.

   */
   void onStashesHeaderClicked();
   /*!
    \brief Expands or contracts the submodules list widget.

   */
   void onSubmodulesHeaderClicked();

   /**
    * @brief onSubtreesHeaderClicked Expands or contracts the subtrees list widget.
    */
   void onSubtreesHeaderClicked();
   /*!
    \brief Gets the SHA for a given tag and notifies the UI that it should select it in the repository view.

    \param item The tag item from the tags list.
   */
   void onTagClicked(QTreeWidgetItem *item);
   /*!
    \brief Gets the SHA for a given stash and notifies the UI that it should select it in the repository view.

    \param item The stash item from the stashes list.
   */
   void onStashClicked(QListWidgetItem *item);

   /**
    * @brief onStashSelected Gets the SHA for a given stash and notifies the UI that it should select it in the
    * repository view.
    * @param stash The stash name.
    */
   void onStashSelected(const QString &stashId);

   /**
    * @brief onSearchBranch Searches for a branch in the children BranchTreeWidget.
    */
   void onSearchBranch();

   QPair<QString, QString> getSubtreeData(const QString &prefix);
};
