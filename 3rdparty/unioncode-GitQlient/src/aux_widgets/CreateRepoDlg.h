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

class GitConfig;

namespace Ui
{
class CreateRepoDlg;
}

/**
 * @brief Defines how the CreateRepoDlg behaviour will be configured.
 *
 * @enum CreateRepoDlgType
 */
enum class CreateRepoDlgType
{
   CLONE,
   INIT
};

/**
 * @brief The CreateRepoDlg creates a dialog to handle repositories when they are not already cloned. The dialog can be
 * configured in two different ways:
 * - Clone: The dialog will clone a repository based on its URL and a selected destination folder.
 * - Init: The dialog will init a new repository that doesn't have a mirror remotely.
 *
 * @class CreateRepoDlg CreateRepoDlg.h "CreateRepoDlg.h"
 */
class CreateRepoDlg : public QDialog
{
   Q_OBJECT

signals:
   /**
    * @brief Signal emitted when the user accepts the configuration to notify other widgets that that the user wants to
    * open the repo after init or clone it.
    *
    * @param path The path to the new repository.
    */
   void signalOpenWhenFinish(const QString &path);

public:
   /**
    * @brief Default constructor that takes the configuration to init or clone a new repository.
    *
    * @param type Defines how the dialog is configured.
    * @param git The Git object to perform repository operations.
    * @param parent The parent widget if needed.
    */
   explicit CreateRepoDlg(CreateRepoDlgType type, QSharedPointer<GitConfig> git, QWidget *parent = nullptr);
   /**
    * @brief Destructor of the class.
    */
   ~CreateRepoDlg() override;

   /**
    * @brief Once all the validations ar passed, the dialog performs the configured action.
    */
   void accept() override;

private:
   Ui::CreateRepoDlg *ui;
   CreateRepoDlgType mType;
   QSharedPointer<GitConfig> mGit;

   /**
    * @brief Opens a file dialog configured to select the destination folder of the repository.
    */
   void selectFolder();
   /**
    * @brief Extracts the project name from the URL when cloning a repository.
    * @param url The url from the remote repository
    */
   void addDefaultName(const QString &url);
   /**
    * @brief Allows the user to configure the local repository email and user name.
    */
   void showGitControls();
};
