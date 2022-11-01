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

class CommitInfoPanel;
class GitBase;
class QPinnableTabWidget;
class IDiffWidget;
class QVBoxLayout;
class FileListWidget;
class GitCache;
class QListWidgetItem;

/*!
 \brief The DiffWidget class creates the layout to display the dif information for both files and commits.

*/
class DiffWidget : public QFrame
{
   Q_OBJECT

signals:
   /*!
    \brief Signal triggered when the user wants to see the history and blame of a given file.

    \param fileName The full file name.
   */
   void signalShowFileHistory(const QString &fileName);
   /*!
    \brief Signal triggered when the user close the last diff opened. This signal is used to disable the button in the
    Controls widget and return to the last view.

   */
   void signalDiffEmpty();

public:
   /*!
    \brief Default constructor.

    \param git The git object to perform Git operations.
    \param cache The internal repository cache for the repository.
    \param parent The parent wiget if needed.
   */
   explicit DiffWidget(const QSharedPointer<GitBase> git, QSharedPointer<GitCache> cache, QWidget *parent = nullptr);
   /*!
    \brief Destructor

   */
   ~DiffWidget() override;

   /*!
    \brief Reloads the information currently shown in the diff.

   */
   void reload();

   /*!
    \brief Clears the information in the current diff.

   */
   void clear() const;
   /*!
    \brief Loads a file diff.

    \param sha The current SHA as base.
    \param previousSha The SHA to compare to.
    \param file The file to show the diff of.
    \return bool Returns true if the file diff was loaded correctly.
   */
   bool loadFileDiff(const QString &sha, const QString &previousSha, const QString &file, bool isCached);
   /*!
    \brief Loads a full commit diff.

    \param sha The base SHA.
    \param parentSha The SHA to compare to.
    \return True if the load was successful, otherwise false.
   */
   bool loadCommitDiff(const QString &sha, const QString &parentSha);

private:
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitCache> mCache;
   CommitInfoPanel *mInfoPanelBase = nullptr;
   CommitInfoPanel *mInfoPanelParent = nullptr;
   QPinnableTabWidget *mCenterStackedWidget = nullptr;
   QMap<QString, IDiffWidget *> mDiffWidgets;
   FileListWidget *fileListWidget = nullptr;
   QString mCurrentSha;
   QString mParentSha;

   /*!
    \brief When the user selects a different diff from a different tab, it changes the information in the commit info
    panel.

    \param index The new selected index.
   */
   void changeSelection(int index);

   /**
    * @brief onTabClosed Removes the IDiffWidget from the map.
    * @param index The index to be closed.
    */
   void onTabClosed(int index);

   void onDoubleClick(QListWidgetItem *item);
};
