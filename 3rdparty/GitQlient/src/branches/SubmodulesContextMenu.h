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

#include <QMenu>

class GitBase;

/*!
 \brief The SubmodulesContextMenu shows the different options that can be performed to the stashes.

*/
class SubmodulesContextMenu : public QMenu
{
   Q_OBJECT

signals:
   /*!
    \brief Signal triggered if a submodule wants to be opened.

   */
   void openSubmodule(const QString &path);
   /*!
    \brief Signal triggered when the main GitQlient UI needs to be updated because of a change in the stashes.

   */
   void infoUpdated();

public:
   /*!
    \brief Default constructor.

    \param git The git object to perform Git operations.
    \param stashId The stash ID that will be used to perform operations into.
    \param parent The parent widget if needed.
   */
   explicit SubmodulesContextMenu(const QSharedPointer<GitBase> &git, const QModelIndex &index,
                                  QWidget *parent = nullptr);

   explicit SubmodulesContextMenu(const QSharedPointer<GitBase> &git, const QString &name, QWidget *parent = nullptr);

private:
   QSharedPointer<GitBase> mGit;
};
