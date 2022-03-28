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

class GitBase;

namespace Ui
{
class TagDlg;
}

/*!
 \brief The TagDlg class helps the user to create both local and remote tags in the repository.

*/
class TagDlg : public QDialog
{
   Q_OBJECT

public:
   /*!
    \brief The default constructor.

    \param git The git object to perform Git operations.
    \param sha The sha where the tag will be created.
    \param parent The parent widget if needed.
   */
   explicit TagDlg(const QSharedPointer<GitBase> &git, const QString &sha, QWidget *parent = nullptr);
   /*!
    \brief Deleted copy constructor

   */
   TagDlg(const TagDlg &) = delete;
   /*!
    \brief Deleted assignment operator.

    \return TagDlg &operator A new TagDlg object.
   */
   TagDlg &operator=(const TagDlg &) = delete;
   /*!
    \brief Destructor.

   */
   ~TagDlg() override;

   /*!
    \brief Validates the data input by the user and performs the git add tag action.

   */
   void accept() override;

private:
   Ui::TagDlg *ui;
   QSharedPointer<GitBase> mGit;
   QString mSha;
};
