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

#include <QProgressDialog>

/**
 * @brief This is a re-implementation of the QProgressDialog to better suite styles and special behaviour flags.
 *
 * @class ProgressDlg ProgressDlg.h "ProgressDlg.h"
 */
class ProgressDlg : public QProgressDialog
{
public:
   /**
    * @brief Default constructor of the class.
    *
    * @param labelText The text that will be shown.
    * @param cancelButtonText Cancel button text.
    * @param minimum Minimum value.
    * @param maximum Maximum value.
    * @param autoReset Tells the progress dialog if it should automatically reset.
    * @param autoClose Tells the progress dialog if it should automatically close.
    */
   explicit ProgressDlg(const QString &labelText, const QString &cancelButtonText, int maximum, bool autoClose);

   /**
    * @brief Detects the press event to avoid closing the dialog when the Esc key is pressed.
    *
    * @param e The press event
    */
   void keyPressEvent(QKeyEvent *e) override;
   /**
    * @brief Detects the close event to filter the close event and only close the dialog if the user clicked on the
    * button.
    *
    * @param e The close event
    */
   void closeEvent(QCloseEvent *e) override;
   /**
    * @brief Closes the dialog by user's action.
    */
   void close();

private:
   bool mPrepareToClose = false;
};
