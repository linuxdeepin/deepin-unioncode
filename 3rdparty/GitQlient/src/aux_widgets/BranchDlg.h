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

#include <QDialog>

namespace Ui
{
class BranchDlg;
}

class GitCache;
class GitBase;

/**
 * @brief The mode indicates what operation will perform the dialog
 *
 * @enum BranchDlgMode
 */
enum class BranchDlgMode
{
   CREATE,
   CREATE_CHECKOUT,
   CREATE_FROM_COMMIT,
   CREATE_CHECKOUT_FROM_COMMIT,
   RENAME,
   STASH_BRANCH,
   PUSH_UPSTREAM
};

/**
 * @brief The configuration indicates the dialog which is the base branch and in which mode the dialog has to be
 * executed.
 *
 * @class BranchDlgConfig BranchDlg.h "BranchDlg.h"
 */
struct BranchDlgConfig
{
   /**
    * @brief mCurrentBranchName The current working branch.
    */
   QString mCurrentBranchName;
   /**
    * @brief mDialogMode The dialog mode configuration.
    */
   BranchDlgMode mDialogMode;
   QSharedPointer<GitCache> mCache;
   /**
    * @brief mGit The GitBase object to perform the git branch operations.
    */
   QSharedPointer<GitBase> mGit;
};

/**
 * @brief The BranchDlg class creates a small dialog where the user can apply branch operations as: create (with
 * different newonces), rename, stash a branch and push a local branch.
 *
 * For the creation of a branch the user can configure the dialog in different ways:
 * - Normal create: creates a new branch in the same commit that the branch selected
 * - Create and checkout: in addition to create a branch, GitQlient will checkout the new branch to continue the work in
 * there.
 * - Create from commit: instead of taking a branch as reference, the dialog takes a commit SHA to create a new branch
 * in there.
 *
 * The operations over Git are done once the user accepts the dialog and all the validations pass.
 *
 * @class BranchDlg BranchDlg.h "BranchDlg.h"
 */
class BranchDlg : public QDialog
{
   Q_OBJECT

public:
   /**
    * @brief Constructor that takes the configuration as parameter. Builds the UI layout and configures some widgets
    * based on the configuration.
    *
    * @param config The configuration for the dialog
    * @param parent The parent widget if needed
    */
   explicit BranchDlg(BranchDlgConfig config, QWidget *parent = nullptr);
   /**
    * @brief Destructor that deallocates the Ui::BranchDlg class.
    */
   ~BranchDlg() override;

private:
   Ui::BranchDlg *ui = nullptr;
   BranchDlgConfig mConfig;

   /**
    * @brief Validates that the new branch name is not the same for the creation and renaming cases. If the user is
    * trying to push the branch it won't validate it's name.
    */
   void checkNewBranchName();
   /**
    * @brief Executes the Git actions based on the configuration once the validation as taken place.
    */
   void accept() override;

   /**
    * @brief copyBranchName Copies the current remote branch name into the line edit so the user doesn't have to type
    * it.
    */
   void copyBranchName();
};
