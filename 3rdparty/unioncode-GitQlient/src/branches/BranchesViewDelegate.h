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

#include <QStyledItemDelegate>

class QPainter;

/*!
 \brief BranchesViewDelegate is the delegate in charge of painting how the branches are display in the BranchTreeWidget.

*/
class BranchesViewDelegate : public QStyledItemDelegate
{
public:
   /*!
    \brief Default constructor.

    \param parent The parent widget if needed.
   */
   explicit BranchesViewDelegate(bool isTag = false, QObject *parent = nullptr);

   /*!
    \brief Overridden paint method used to display different colors when mouse actions happen.

    \param p The painter device.
    \param o The style options of the item.
    \param i The item data
   */
   void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const override;
   /*!
    \brief Overridden method that returns the size of the row, both height and width.

    \return QSize The width and height of the row.
   */
   QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;

private:
   bool mIsTag = false;
};
