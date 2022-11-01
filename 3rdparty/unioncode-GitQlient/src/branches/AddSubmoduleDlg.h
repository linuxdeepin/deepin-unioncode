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

class GitSubmodules;

namespace Ui
{
class AddSubmoduleDlg;
}

/**
 * @brief AddSubmoduleDlg creates a dialog for the user to add a new submodule in the current repository.
 *
 */
class AddSubmoduleDlg : public QDialog
{
   Q_OBJECT

public:
   /**
    * @brief Default constructor.
    *
    * @param git The git object to execute Git commands.
    * @param parent The parent widget if needed.
    */
   explicit AddSubmoduleDlg(const QSharedPointer<GitSubmodules> &git, QWidget *parent = nullptr);
   /**
    * @brief Destructor.
    *
    */
   ~AddSubmoduleDlg() override;

   /**
    * @brief When the user clicks the Ok/Accept button, it triggers the \ref accept method that validates the data and
    * tries to perform the Git action.If it's successfully executed, it will close the dialog.
    *
    */
   void accept() override;

private:
   Ui::AddSubmoduleDlg *ui;
   QSharedPointer<GitSubmodules> mGit;
};
