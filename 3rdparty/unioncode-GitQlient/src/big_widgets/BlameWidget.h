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
#include <QMap>

class GitCache;
class GitBase;
class QFileSystemModel;
class FileBlameWidget;
class QTreeView;
class CommitHistoryModel;
class CommitHistoryView;
class QTabWidget;
class QModelIndex;
class RepositoryViewDelegate;
class GitQlientSettings;

/**
 * @brief The BlameWidget class creates the layout that contains all the widgets that are part of the blame and history
 * view. The blame&history view is formed by a view that shows the history view of a given file, another view that shows
 * the files in the repository folder, and finally a central widget in the form of a QTabWidget that openes as many file
 * blames as the user wants.
 *
 * After the widget is instantiated and before its first use, it needs to be initialized by calling the @ref init
 * method. Once it's done, it can open files requested by other widgets by using the @p showFileHistory method, that
 * takes the file path.
 *
 * Internally the class also opens files but by taking the index from the QFileSystemModel.
 *
 */
class BlameWidget : public QFrame
{
   Q_OBJECT

signals:
   /**
    * @brief Signal triggered when the user wants to show the diff of file between two commits.
    *
    * @param sha The current sha
    * @param parentSha The previous sha
    * @param file The full path of the file to diff
    */
   void showFileDiff(const QString &sha, const QString &parentSha, const QString &file, bool isCached);

   /**
    * @brief Signal triggered when the user wants to see the diff of the selected SHA compared to its previous one.
    * @param shas The selected commit SHA and its previous one.
    */
   void signalOpenDiff(const QStringList &shas);

public:
   /**
    * @brief Constructor.
    *
    * @param cache The GitQlient cache for the current repository.
    * @param git The Git object to execute git commands.
    * @param parent The parent widget if needed.
    */
   explicit BlameWidget(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                        const QSharedPointer<GitQlientSettings> &settings, QWidget *parent = nullptr);
   /**
    * @brief Destructor.
    *
    */
   ~BlameWidget();

   /**
    * @brief The init method configures the file system view and sets the current working directory.
    *
    * @param workingDirectory The current working directory.
    */
   void init(const QString &workingDirectory);

   /**
    * @brief Opens the blame for a given file. This method configures both the history view, where the user can check
    * all the commits where this file has been modified and also adds a tab in the central QTabWidget.
    *
    * @param filePath The full file path.
    */
   void showFileHistory(const QString &filePath);
   /**
    * @brief Configures the repository model once git finished to load the repository.
    *
    * @param totalCommits The total of commits loaded.
    */
   void onNewRevisions(int totalCommits);

private:
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitQlientSettings> mSettings;
   QFileSystemModel *fileSystemModel = nullptr;
   CommitHistoryModel *mRepoModel = nullptr;
   CommitHistoryView *mRepoView = nullptr;
   QTreeView *fileSystemView = nullptr;
   QTabWidget *mTabWidget = nullptr;
   QString mWorkingDirectory;
   QMap<QString, FileBlameWidget *> mTabsMap;
   RepositoryViewDelegate *mItemDelegate = nullptr;
   int mSelectedRow = -1;
   int mLastTabIndex = 0;

   /**
    * @brief Opens the blame for a given index from the file system model. This method configures both the history view,
    * where the user can check all the commits where this file has been modified and also adds a tab in the central
    * QTabWidget.
    *
    * @param index The index from the file system model.
    */
   void showFileHistoryByIndex(const QModelIndex &index);
   /**
    * @brief Shows the context menu for the history view.
    *
    * @param pos The position where the menu should be shown.
    */
   void showRepoViewMenu(const QPoint &pos);
   /**
    * @brief This method reloads a blame when the user selects a different commit from the history view. In the history
    * view the user can select newer or older commits where the file was modified. When it selects a different commit,
    * both the selected SHA and its previous SHA are sent to reload the blame view.
    *
    * @param index The index from the history view.
    */
   void reloadBlame(const QModelIndex &index);
   /**
    * @brief When the user changes the blame view, the history view is notified to reload its history to accommodate the
    * new information from the new selected file.
    *
    * @param tabIndex The new tab index selected.
    */
   void reloadHistory(int tabIndex);

   /*!
     \brief Retrieves the SHA from the QModelIndex and triggers the \ref signalOpenDiff signal.

     \param index The index from the model.
    */
   void openDiff(const QModelIndex &index);
};
