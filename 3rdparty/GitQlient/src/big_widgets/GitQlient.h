#pragma once

/****************************************************************************************
 ** GitQlient is an application to manage and operate one or several Git repositories. With
 ** GitQlient you will be able to add commits, branches and manage all the options Git provides.
 ** Copyright (C) 2021  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This library is free software; you can redistribute it and/or
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

#include <QSet>
#include <QWidget>

class QPinnableTabWidget;
class InitScreen;
class ProgressDlg;
class GitConfig;
class QStackedLayout;
class GitQlientSettings;
class GitBase;

/*!
 \brief The GitQlient class is the MainWindow of the GitQlient application. Is the widget that stores all the tabs about
 the opened repositories and their submodules. Acts as a bridge between the repository actions performed by the
 ConfigWidget and the GitQlientWidget.

*/
class GitQlient : public QWidget
{
   Q_OBJECT

public:
   /*!
    \brief Default constructor. Creates an empty GitQlient instance.

    \param parent The parent widget if needed.
   */
   explicit GitQlient(QWidget *parent = nullptr);

   /*!
    \brief Destructor.

   */
   ~GitQlient() override;

   /*!
    \brief Set the repositories that will be shown.

    \param repositories
   */
   void setRepositories(const QStringList &repositories);
   /*!
    \brief In case that the GitQlient instance it's already initialize, the user can add arguments to be processed.

    \param arguments The list of arguments.
    \return Returns true if application should continue or false if it should quit.
   */
   bool setArgumentsPostInit(const QStringList &arguments);

   /**
    * @brief restorePinnedRepos This method restores the pinned repos from the last session
    * @param pinnedRepos The list of repos to restore
    */
   void restorePinnedRepos();

   /*!
    \brief This method parses all the arguments and configures GitQlient settings with them. Part of the arguments can
    be a list of repositories to be opened. In that case, the method returns the list of repositories to open in the
    repos out parameter.

    \param arguments Arguments from the command prompt.
    \param repos Output paramter, repositories to open.
    \return Returns true if application should continue or false if it should quit.
   */
   static bool parseArguments(const QStringList &arguments, QStringList *repos);

protected:
   bool eventFilter(QObject *obj, QEvent *event) override;

private:
   QStackedLayout *mStackedLayout = nullptr;
   QPinnableTabWidget *mRepos = nullptr;
   InitScreen *mConfigWidget = nullptr;
   QSet<QString> mCurrentRepos;
   QSharedPointer<GitConfig> mGit;
   ProgressDlg *mProgressDlg = nullptr;
   QString mPathToOpen;

   /*!
    \brief Opens a QFileDialog to select a repository in the local disk.
   */
   void openRepo();

   /**
    * @brief Opens a QFileDialog to select a repository in the local disk.
    * @param path The path of the new repo.
    */
   void openRepoWithPath(const QString &path);

   /*!
    \brief Clones a new repository.
   */
   void cloneRepo();

   /*!
    \brief Initiates a new local repository.
   */
   void initRepo();

   /**
    * @brief Updates the progress dialog for cloning repos.
    *
    * @param stepDescription The description step.
    * @param value The numeric value.
    */
   void updateProgressDialog(QString stepDescription, int value);

   /**
    * @brief showError Shows an error occurred during any configuration time.
    * @param error The error code.
    * @param description The error description.
    */
   void showError(int error, QString description);

   /*!
    \brief Creates a new GitQlientWidget instance or the repository defined in the \p repoPath value. After that, it
    adds a new tab in the current widget.

    \param repoPath The full path of the repository to be opened.
   */
   void addRepoTab(const QString &repoPath);

   /*!
    \brief Creates a new GitQlientWidget instance or the repository defined in the \p repoPath value. After that, it
    adds a new tab in the current widget.

   \param repoPath The full path of the repository to be opened.
           */
   void addNewRepoTab(const QString &repoPath, bool pinned);
   /*!
    \brief Closes a tab. This implies to close all child widgets and remove cache and configuration for that repository
    until it's opened again.

    \param tabIndex The tab index that triggered the close action.
   */
   void closeTab(int tabIndex);

   /**
    * @brief onSuccessOpen Refreshes the UI for the most used and most recent projects lists.
    * @param fullPath The full path of the project successfully opened.
    */
   void onSuccessOpen(const QString &fullPath);

   /**
    * @brief conditionallyOpenPreConfigDlg Opens the pre-config dialog in case that the repo is open for the very first
    * time.
    * @param git The git object to perform Git operations.
    * @param settings The settings object to store the new values.
    */
   void conditionallyOpenPreConfigDlg(const QSharedPointer<GitBase> &git,
                                      const QSharedPointer<GitQlientSettings> &settings);

   /**
    * @brief updateWindowTitle Updates the window title of GitQlient appending the branch of the current repository.
    * @param currentTabIndex The current tab index used to retrieve the repository.
    */
   void updateWindowTitle();
};
