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

class QToolButton;
class QPushButton;
class GitBase;
class GitCache;
class GitTags;
class GitCache;
class QNetworkAccessManager;
class QProgressBar;
class GitQlientUpdater;
class QButtonGroup;
class QHBoxLayout;
class PomodoroButton;

/*!
 \brief Enum used to configure the different views handled by the Controls widget.

*/
enum class ControlsMainViews
{
   History,
   Diff,
   Blame,
   Merge,
   GitServer,
   BuildSystem,
   Config
};

/*!
 \brief The Controls class creates the layout to store all the buttons that acts over the UI current view and the most
 used Git actions.

*/
class Controls : public QFrame
{
   Q_OBJECT

signals:
   /*!
    \brief Signal triggered when the user wants to go to the main repository view.

   */
   void signalGoRepo();
   /*!
    \brief Signal triggered when the user selects the diff view.

   */
   void signalGoDiff();
   /*!
    \brief Signal triggered when the user selects the Blame&History view.

   */
   void signalGoBlame();
   /*!
    \brief Signal triggered when the user selects the merge conflict resolution view.

   */
   void signalGoMerge();

   /**
    * @brief signalGoManagement Signal triggered when the user selected the Git remote platform view.
    */
   void signalGoServer();

   /**
    * @brief signalGoBuildSystem Signal triggered when the user selected the Build System view.
    */
   void signalGoBuildSystem();
   /*!
    * \brief Signal triggered when trying to pull and a conflict happens.
    */
   void signalPullConflict();

   /**
    * @brief signalRefreshPRsCache Signal that refreshes PRs cache.
    */
   void signalRefreshPRsCache();

   /**
    * @brief requestReload Signal triggered when the user forces a full refresh of the repository data.
    */
   void requestFullReload();

   /**
    * @brief requestReload Signal triggered when the user forces a refresh of the references of the repository.
    */
   void requestReferencesReload();

   /**
    * @brief goConfig Signal triggered when the user seleced the config view.
    */
   void goConfig();

public:
   /*!
    \brief Default constructor.

    \param git The git object to perform Git operations.
    \param parent The parent widget if needed.
   */
   explicit Controls(const QSharedPointer<GitCache> &cache, const QSharedPointer<GitBase> &git,
                     QWidget *parent = nullptr);

   /**
    * @brief Destructor.
    */
   ~Controls();
   /*!
    \brief Process the toggled button and triggers its corresponding action.

    \param view The view the user selected.
   */
   void toggleButton(ControlsMainViews view);
   /*!
    \brief Sets the current SHA.

    \param sha The SHA hash.
   */
   void setCurrentSha(const QString &sha) { mCurrentSha = sha; }
   /*!
    \brief Set all the buttons as enabled.

    \param enabled True to enable, false otherwise.
   */
   void enableButtons(bool enabled);
   /*!
    \brief Performs the fetch action. Can be triggered manually or by a timer.

   */
   void fetchAll();
   /*!
    \brief Activates the merge warning frame.

   */
   void activateMergeWarning();
   /*!
    \brief Disables the merge warning frame.

   */
   void disableMergeWarning();
   /*!
    \brief Disables the diff button and view.

   */
   void disableDiff();
   /*!
    \brief Enables the diff button and view.

   */
   void enableDiff();
   /*!
    \brief Gets the current selected button/view.

    \return ControlsMainViews The value of the current selected button.
   */
   ControlsMainViews getCurrentSelectedButton() const;

private:
   QString mCurrentSha;
   QSharedPointer<GitCache> mCache;
   QSharedPointer<GitBase> mGit;
   QSharedPointer<GitTags> mGitTags;
   QToolButton *mHistory = nullptr;
   QToolButton *mDiff = nullptr;
   QToolButton *mBlame = nullptr;
   QToolButton *mPullBtn = nullptr;
   QToolButton *mPullOptions = nullptr;
   QToolButton *mPushBtn = nullptr;
   QToolButton *mRefreshBtn = nullptr;
   QToolButton *mConfigBtn = nullptr;
   QToolButton *mGitPlatform = nullptr;
   QToolButton *mBuildSystem = nullptr;
   PomodoroButton *mPomodoro = nullptr;
   QToolButton *mVersionCheck = nullptr;
   QPushButton *mMergeWarning = nullptr;
   GitQlientUpdater *mUpdater = nullptr;
   QButtonGroup *mBtnGroup = nullptr;
   bool mGoGitServerView = false;

   /*!
    \brief Pulls the current branch.

   */
   void pullCurrentBranch();
   /*!
    \brief Pushes the current local branch changes.

   */
   void pushCurrentBranch();
   /*!
    \brief Prunes all branches, tags and stashes.

   */
   void pruneBranches();

   /**
    * @brief createGitPlatformButton Createst the git platform button if the user has enabled it.
    */
   void createGitPlatformButton(QHBoxLayout *layout);

   /**
    * @brief createBuildSystemButton Creates the build system platform button if the user has enabled it.
    */
   void configBuildSystemButton();

   bool eventFilter(QObject *obj, QEvent *event);
};
