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

class CommitInfo;
class QLabel;
class QScrollArea;
class ButtonLink;

/**
 * @brief The CommitInfoPanel class represents the basic information of a commit. It's presented in the form of a simple
 * UI frame with a heading on top that shows the commit SHA and followed by the commit title and description. Later on,
 * the information about the commiteer is presented.
 */
class CommitInfoPanel : public QFrame
{
public:
   /**
    * @brief CommitInfoPanel Default constructor.
    * @param parent The parent widget.
    */
   explicit CommitInfoPanel(QWidget *parent = nullptr);

   /**
    * @brief configure Configures the panel with the information of the given @p commit.
    * @param commit The commit to get the data from.
    */
   void configure(const CommitInfo &commit);
   /**
    * @brief clear Clears all the widgets data.
    */
   void clear();

private:
   ButtonLink *mLabelSha = nullptr;
   QLabel *mLabelTitle = nullptr;
   QLabel *mLabelDescription = nullptr;
   QScrollArea *mScrollArea = nullptr;
   QLabel *mLabelAuthor = nullptr;
   QLabel *mLabelDateTime = nullptr;
};
