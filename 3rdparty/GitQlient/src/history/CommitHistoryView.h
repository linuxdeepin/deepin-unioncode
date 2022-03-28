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

#include <QTreeView>

class GitCache;
class GitBase;
class CommitHistoryModel;
class ShaFilterProxyModel;
class GitServerCache;
class GitQlientSettings;

/**
 * @brief The CommitHistoryView is the class that represents the View in a MVC pattern. It shows the data provided by
 * the model regarding the repository graph and the commit information.
 *
 * @class CommitHistoryView CommitHistoryView.h "CommitHistoryView.h"
 */
class CommitHistoryView : public QTreeView
{
   Q_OBJECT

signals:
   void fullReload();
   void referencesReload();
   void logReload();

   /*!
    \brief Signal triggered when the user wants to open the diff of a commit compared to its parent.

    \param sha The SHA to diff.
   */
   void signalOpenDiff(const QString &sha);
   /*!
    \brief Signal triggered when the user whants to diff the shas in the list. This signal is only emited if the user
    selected two SHAs.

    \param sha The shas to diff between.
   */
   void signalOpenCompareDiff(const QStringList &sha);
   /*!
    \brief Signal triggered when the user wants to amend a commit.

    \param sha The SHA of the commit to amend.
   */
   void signalAmendCommit(const QString &sha);
   /*!
    \brief Signal triggered when a merge has been requested. Since it involves a lot of changes at UI level this action
    is not performed here.

    \param origin The branch to merge from.
    \param destination The branch to merge into.
   */
   void signalMergeRequired(const QString &origin, const QString &destination);

   /**
    * @brief Signal triggered when a merge with squash behavior has been requested. Since it involves a lot of changes
    * at UI level this action is not performed here.
    *
    * @param origin The branch to merge from.
    * @param destination The branch to merge into.
    */
   void mergeSqushRequested(const QString &origin, const QString &destination);

   /*!
    * \brief signalConflict Signal triggered when trying to cherry-pick or pull and a conflict happens.
    */
   void signalCherryPickConflict(const QStringList &pendingShas);
   /*!
    * \brief signalPullConflict Signal triggered when trying to pull and a conflict happens.
    */
   void signalPullConflict();
   /**
    * @brief showPrDetailedView Signal that makes the view change to the Pull Request detailed view
    * @param pr The pull request number to show.
    */
   void showPrDetailedView(int pr);

public:
   /**
    * @brief Default constructor.
    *
    * @param cache The internal cache for the current repository.
    * @param git The git object to perform Git commands.
    * @param parent The parent widget if needed.
    */
   explicit CommitHistoryView(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                              const QSharedPointer<GitQlientSettings> &settings,
                              const QSharedPointer<GitServerCache> &gitServerCache, QWidget *parent = nullptr);
   /**
    * @brief Destructor.
    */
   ~CommitHistoryView() override;

   /**
    * @brief Sets the model that will be used by the view.
    *
    * @param model Model to set into the view.
    */
   void setModel(QAbstractItemModel *model) override;
   /**
    * @brief Returns the list of SHAs that the user has selected in the view.
    *
    * @return QStringList Gets the selected SHA list.
    */
   QStringList getSelectedShaList() const;
   /**
    * @brief If the view has a filter active this method tells the filter which SHAs are going to be shown.
    *
    * @param shaList List of SHA to pass to the filter.
    */
   void filterBySha(const QStringList &shaList);
   /**
    * @brief Activates/deactivates filtering in the view.
    *
    * @param activate True to activate the filter. Otherwise false,
    */
   void activateFilter(bool activate) { mIsFiltering = activate; }
   /**
    * @brief Tells if the user has any active filter.
    *
    * @return bool Returns true if the widget is actively filtering. Otherwise, false.
    */
   bool hasActiveFilter() const { return mIsFiltering; }

   /**
    * @brief Clears any selection or data in the view.
    */
   void clear();
   /**
    * @brief Puts the focus (and selects) the given SHA.
    *
    * @param goToSha The SHA to select.
    */
   void focusOnCommit(const QString &goToSha);
   /**
    * @brief Gets the current selected SHA.
    *
    * @return QString Returns the current selected SHA.
    */
   QString getCurrentSha() const { return mCurrentSha; }
   /**
    * @brief Overridden function to make it public. Useful to retrieve the indexes when a search is done.
    *
    * @return QModelIndexList The list of selected indexes.
    */
   QModelIndexList selectedIndexes() const override;

private:
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitQlientSettings> mSettings;
   QSharedPointer<GitServerCache> mGitServerCache;
   CommitHistoryModel *mCommitHistoryModel = nullptr;
   ShaFilterProxyModel *mProxyModel = nullptr;
   bool mIsFiltering = false;
   QString mCurrentSha;

   /**
    * @brief Shows the context menu for the CommitHistoryView.
    *
    * @param p The point where the context menu will be shown.
    */
   void showContextMenu(const QPoint &p);
   /**
    * @brief Configures the tree view and how the columns look like.
    *
    * @fn setupGeometry
    */
   void setupGeometry();
   /**
    * @brief Stores the new selected SHA.
    *
    * @param index The index that changed. Used to retrieve the row.
    * @param parent The parent of the index. Not used.
    */
   void currentChanged(const QModelIndex &index, const QModelIndex &parent) override;
   /**
    * @brief refreshView Refreshes the view.
    */
   void refreshView();
   /**
    * @brief onHeaderContextMenu Shows the context menu for the header of the tree view.
    * @param pos The position of the cursor that will be used to show the menu.
    */
   void onHeaderContextMenu(const QPoint &pos);
};
