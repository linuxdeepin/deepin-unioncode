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

#include <QSortFilterProxyModel>

/**
 * @brief The ShaFilterProxyModel class is an overload of the QSortFilterProxyModel that takes a list of shas to act as
 * a filter between a view and a QAbstractiItemModel.
 *
 */
class ShaFilterProxyModel : public QSortFilterProxyModel
{
   Q_OBJECT

public:
   /**
    * @brief Default constructor.
    *
    * @param parent The parent widget if needed.
    */
   explicit ShaFilterProxyModel(QObject *parent = nullptr);

   /**
    * @brief Sets the list of accepted SHAs that will be shown in the source model.
    *
    * @param acceptedShaList The SHAs list.
    */
   void setAcceptedSha(const QStringList &acceptedShaList) { mAcceptedShas = acceptedShaList; }
   /**
    * @brief Starts the reset of the model
    *
    */
   void beginResetModel() { QSortFilterProxyModel::beginResetModel(); }
   /**
    * @brief Ends the reset of the model.
    *
    */
   void endResetModel() { QSortFilterProxyModel::endResetModel(); }

protected:
   /**
    * @brief This method is the actual filter functionality. Given the source row and the source index it retrieves the
    * sha and checks if it's among the accepted SHAs in the list.
    *
    * @param sourceRow The source row number.
    * @param sourceParent The source index.
    * @return bool Returns true if the source row should be shown in the view.
    */
   bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
   /**
    * @brief mAcceptedShas List of accepted shas.
    */
   QStringList mAcceptedShas;
};
