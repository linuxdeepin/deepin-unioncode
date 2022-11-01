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
class CommitHistoryModel;
class CommitHistoryView;
class QLineEdit;
class BranchesWidget;
class QStackedWidget;
class CommitChangesWidget;
class CommitInfoWidget;
class CheckBox;
class RepositoryViewDelegate;
class FullDiffWidget;
class FileDiffWidget;
class BranchesWidgetMinimal;
class QPushButton;
class GitServerCache;
class QLabel;
class GitQlientSettings;
class QSplitter;
struct GitExecResult;

/*!
 \brief The HistoryWidget is responsible for showing the history of the repository. It is the first widget shown
 when a repository is open and manages all the signals from its subwidgets to the GitQlientRepo class. It also creates
 the layout and acts as a bridge to transfer the signals from one subwidget to the other.

 The layout is formed by four important widgets:
 - In the left side two different widgets are shown depending on the conditions. If the user selects the Work In
 Progress (WIP) commit, the WorkInProgressWidget will be shown. In case it's any another commit, the CommitWidgetInfo
 will be shown.
 - In the center is located the repository view with the graph tree and the commits information.
 - In the right side the BranchWidget is shown where the user has control of everything related with branches, tags,
 stashes and submodules.

*/
class HistoryWidget : public QFrame
{
   Q_OBJECT

signals:
   void fullReload();

   void referencesReload();

   void logReload();

   /*!
    \brief Signal triggered when the user wants to see the diff of the selected SHA compared to its first parent.

    \param sha The selected commit SHA.
   */
   void signalOpenDiff(const QString &sha);
   /*!
    \brief Signal triggered when the user wants to see the diff between two different commits.

    \param sha The list of SHAs to compare.
   */
   void signalOpenCompareDiff(const QStringList &sha);
   /*!
    \brief Signal triggered when the user opens a new submodule. It is necessary to propagate this signal since is the
    GitQlient class the responsible of opening a new tab for the submodule.

    \param submodule The submodule to be opened.
   */
   void signalOpenSubmodule(const QString &submodule);
   /*!
    \brief Signal triggered when the user wants to see the diff of a file between two commits.

    \param sha The base commit SHA.
    \param parentSha The commit SHA to compare to.
    \param fileName The file name for the diff.
   */
   void signalShowDiff(const QString &sha, const QString &parentSha, const QString &fileName, bool isCached);

   /*!
    \brief Signal triggered when changes are committed.

    \param commited True if there was no error, false otherwise.
   */
   void changesCommitted();
   /*!
    \brief Signal triggered when the user wants to see the History & Blame for a specific file.

    \param fileName The file name.
   */
   void signalShowFileHistory(const QString &fileName);
   /*!
    \brief Signal triggered when toggles the option of seeing all the branches in the repository graph view.

    \param showAll True to show all the branches, false if only the current branch must be shown.
   */
   void signalAllBranchesActive(bool showAll);
   /*!
    \brief Signal triggered when the user performs a merge and it contains conflicts.
   */
   void signalMergeConflicts();
   /*!
    * \brief signalConflict Signal triggered when trying to cherry-pick or pull and a conflict happens.
    */
   void signalCherryPickConflict(const QStringList &pendingShas);
   /*!
    * \brief signalPullConflict Signal triggered when trying to pull and a conflict happens.
    */
   void signalPullConflict();
   /*!
    \brief Signal triggered when the WIP needs to be updated.
   */
   void signalUpdateWip();
   /**
    * @brief showPrDetailedView Signal that makes the view change to the Pull Request detailed view
    * @param pr The pull request number to show.
    */
   void showPrDetailedView(int pr);

   /**
    * @brief panelsVisibilityChanged Signal triggered whenever the visibility of the panels in the BranchesWidget
    * changes.
    */
   void panelsVisibilityChanged();

public:
   /*!
    \brief Default constructor.

    \param cache The internal repository cache.
    \param git The git object to perform Git operations.
    \param parent The parent widget if needed.
   */
   explicit HistoryWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> git,
                          const QSharedPointer<GitServerCache> &gitServerCache,
                          const QSharedPointer<GitQlientSettings> &settings, QWidget *parent = nullptr);
   /*!
    \brief Destructor.

   */
   ~HistoryWidget();
   /*!
    \brief Clears all the information in the subwidgets.

   */
   void clear();
   /*!
    \brief Resets the WIP widget.

   */
   void resetWip();

   /**
    * @brief loadBranches Loads the information on the branches widget: branches, tags, stashes and submodules.
    * @brief fullReload Indicates that the information on the panel should be fully reloaded, otherwise only refreshes
    * the link to the current branch.
    */
   void loadBranches(bool fullReload);

   /*!
    \brief If the current view is the WIP widget, updates it.

   */
   void updateUiFromWatcher();
   /*!
    \brief Focuses on the given commit.

    \param sha The commit SHA to focus the view in.
   */
   void focusOnCommit(const QString &sha);
   /*!
    \brief Configures the CommitInfoWidget or the WorkInProgress widget depending on the given SHA. It sets the
    configured widget to visible.

    \param goToSha The SHA to show.
   */
   void selectCommit(const QString &goToSha);

   /*!
    \brief Reloads the history model of the repository graph view when it finishes the loading process.

    \param totalCommits The new total of commits to show in the graph.
   */
   void updateGraphView(int totalCommits);

   /**
    * @brief onCommitTitleMaxLenghtChanged Changes the maximum length of the commit title.
    */
   void onCommitTitleMaxLenghtChanged();

   /**
    * @brief onPanelsVisibilityChaned Reloads the visibility configuration of the panels in the BranchesWidget.
    */
   void onPanelsVisibilityChanged();

protected:
   void keyPressEvent(QKeyEvent *event) override;
   void keyReleaseEvent(QKeyEvent *event) override;

private:
   enum class Pages
   {
      Graph,
      FileDiff,
      FullDiff
   };

   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitServerCache> mGitServerCache;
   QSharedPointer<GitQlientSettings> mSettings;
   CommitHistoryModel *mRepositoryModel = nullptr;
   CommitHistoryView *mRepositoryView = nullptr;
   BranchesWidget *mBranchesWidget = nullptr;
   QLineEdit *mSearchInput = nullptr;
   QStackedWidget *mCommitStackedWidget = nullptr;
   QStackedWidget *mCenterStackedWidget = nullptr;
   CommitChangesWidget *mWipWidget = nullptr;
   CommitChangesWidget *mAmendWidget = nullptr;
   CommitInfoWidget *mCommitInfoWidget = nullptr;
   CheckBox *mChShowAllBranches = nullptr;
   RepositoryViewDelegate *mItemDelegate = nullptr;
   QFrame *mGraphFrame = nullptr;
   FileDiffWidget *mFileDiff = nullptr;
   FullDiffWidget *mFullDiffWidget = nullptr;
   QPushButton *mReturnFromFull = nullptr;
   QLabel *mUserName = nullptr;
   QLabel *mUserEmail = nullptr;
   bool mReverseSearch = false;
   QSplitter *mSplitter = nullptr;

   /*!
    \brief Performs a search based on the input of the search QLineEdit with the users input.

   */
   void search();
   /*!
    \brief Goes to the selected SHA.

    \param sha The selected SHA.
   */
   void goToSha(const QString &sha);
   /*!
    \brief Retrieves the SHA from the QModelIndex from the repository graph model when the user clicks over an item.
    Then, performs the \ref onCommitSelected action.

    \param index The index from the model.
   */
   void commitSelected(const QModelIndex &index);
   /*!
    \brief Action that stores in the settings the new value for the check box to show all the branches. It also triggers
    the \ref signalAllBranchesActive signal.

    \param showAll True to show all branches, false to show only the current branch.
   */
   void onShowAllUpdated(bool showAll);

   /*!
    \brief Opens the AmendWidget.

    \param sha The commit SHA to amend.
   */
   void onAmendCommit(const QString &sha);

   /*!
    \brief Tries to perform the git merge operation from \p branchToMerge into the \p current. If there are conflicts
    the GitQlientRepo class is notified to take actions.

    \param current The current branch.
    \param branchToMerge The branch to merge from.
   */
   void mergeBranch(const QString &current, const QString &branchToMerge);

   /**
    * @brief mergeSquashBranch Tries to perform the git merge operation squashing all commits from \p branchToMerge into
    the \p current. If there are conflicts the GitQlientRepo class is notified to take actions.
    * @param current The current branch
    * @param branchToMerge The branch to merge from
    */
   void mergeSquashBranch(const QString &current, const QString &branchToMerge);

   void processMergeResponse(const GitExecResult &ret);

   /**
    * @brief endEditFile Closes the file diff view.
    */
   void returnToView();

   /**
    * @brief cherryPickCommit Cherry-picks the commit defined by the SHA in the QLineEdit of the filter.
    */
   void cherryPickCommit();

   /**
    * @brief showFileDiff Shows the file diff.
    * @param sha The base commit SHA.
    * @param parentSha The commit SHA to compare with.
    * @param fileName The file name to diff.
    */
   void showFileDiff(const QString &sha, const QString &parentSha, const QString &fileName, bool isCached);

   /**
    * @brief showFileDiff Shows the file diff.
    * @param sha The base commit SHA.
    * @param parentSha The commit SHA to compare with.
    * @param fileName The file name to diff.
    */
   void showFileDiffEdition(const QString &sha, const QString &parentSha, const QString &fileName);

   /**
    * @brief showFullDiff Shows the full commit diff.
    * @param sha The base commit SHA.
    * @param parentSha The commit SHA to compare with.
    */
   void onOpenFullDiff(const QString &sha);

   void rearrangeSplittrer(bool minimalActive);

   void cleanCommitPanels();

   void onRevertedChanges();
};
