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

class QPushButton;
class QButtonGroup;
class GitConfig;
class ProgressDlg;
class GitQlientSettings;
class QVBoxLayout;

/*!
 \brief The ConfigWidget is the widget shown when the user access it from the tool icon. It gives the options of
 initiate, clone and open repositories and also provides the layout to access the general GitQlient configuration.

The widget also shows the list of most open repositories.

*/
class InitScreen : public QFrame
{
   Q_OBJECT

signals:
   /*!
    \brief Signal triggered when the user tries to open a new repository.

    \param repoPath The repository full path.
   */
   void signalOpenRepo(const QString &repoPath);

public:
   /*!
    \brief Default constructor.

    \param parent The parent widget if needed.
   */
   explicit InitScreen(QWidget *parent = nullptr);

   /*!
    * \brief onRepoOpened Increments the uses of the repo in the settings to update the lists.
    *
    * \param repo The absolute path to the repo.
    */
   void onRepoOpened();

private:
   QSharedPointer<GitConfig> mGit;
   QPushButton *mOpenRepo = nullptr;
   QPushButton *mCloneRepo = nullptr;
   QPushButton *mInitRepo = nullptr;
   QButtonGroup *mBtnGroup = nullptr;
   ProgressDlg *mProgressDlg = nullptr;
   QString mPathToOpen;
   QVBoxLayout *mRecentProjectsLayout = nullptr;
   QVBoxLayout *mUsedProjectsLayout = nullptr;
   QWidget *mInnerWidget = nullptr;
   QWidget *mMostUsedInnerWidget = nullptr;

   /*!
    \brief Opens an already cloned repository.

   */
   void openRepo();
   /*!
    \brief Clones a new repository.

   */
   void cloneRepo();
   /*!
    \brief Initiates a new local repository.

   */
   void initRepo();
   /*!
    \brief Creates the recent projects page.

    \return QWidget The resultant widget.
   */
   QWidget *createRecentProjectsPage();

   /*!
    \brief Creates the most used projects page.

   \return QWidget The resultant widget.
    */
   QWidget *createUsedProjectsPage();

   /*!
    \brief Updates the progress dialog for cloning repos.

    \param stepDescription The description step.
    \param value The numeric value.
   */
   void updateProgressDialog(QString stepDescription, int value);

   /**
    * @brief showError Shows an error occurred during any configuration time.
    * @param error The error code.
    * @param description The error description.
    */
   void showError(int error, QString description);

   /**
    * @brief showAbout Shows GitQlient about info.
    */
   void showAbout();

   /**
    * @brief openConfigDlg Opens the config dialog.
    */
   void openConfigDlg();
};
