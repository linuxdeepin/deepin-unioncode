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

#include <QColor>
#include <QScopedPointer>
#include <QString>

#include <array>

class GitQlientSettings;

/*!
 \brief GitQlientStyles contains the information shared between all the instances of GitQlientWidget. This is a general
 GitQlient styles configuration.

*/
class GitQlientStyles
{
private:
   static const int kBranchColors = 8; /*!< Total of branch colors. */

public:
   /*!
    \brief Gets the singleton instance.

    \return GitQlientStyles The instance for the styles.
   */
   static GitQlientStyles *getInstance();
   /*!
    \brief Gets the current stylesheet.

    \return QString The stylesheet.
   */
   static QString getStyles();
   /*!
    \brief Gets the text color.

    \return QColor Current text color.
   */
   static QColor getTextColor();

   /**
    * @brief Gets the row selection color
    * @return QColor Current row selection color
    */
   static QColor getGraphSelectionColor();

   /**
    * @brief Gets the row hover color
    * @return QColor Current row hover color
    */
   static QColor getGraphHoverColor();

   /**
    * @brief Gets the background color
    * @return QColor Current background color
    */
   static QColor getBackgroundColor();

   /**
    * @brief Gets the tabs color
    * @return QColor Current tab color
    */
   static QColor getTabColor();

   /*!
    \brief Gets the GitQlient blue color.

    \return QColor
   */
   static QColor getBlue();
   /*!
    \brief Gets the GitQlient red color.

    \return QColor
   */
   static QColor getRed();
   /*!
    \brief Gets the GitQlient green color.

    \return QColor
   */
   static QColor getGreen();
   /*!
    \brief Gets the GitQlient orange color.

    \return QColor
   */
   static QColor getOrange();
   /*!
    \brief Gets the total count of branch colors.

    \return int
   */
   static int getTotalBranchColors() { return kBranchColors; }
   /*!
    \brief Gets all the branch colors.

    \return std::array<QColor, kBranchColors>
   */
   static std::array<QColor, kBranchColors> getBranchColors();
   /*!
    \brief Gets the branch color for a given \p index.

    \param index The position of the color in the array.
    \return QColor Returns the color.
   */
   static QColor getBranchColorAt(int index);

private:
   static GitQlientStyles *INSTANCE;

   /*!
    \brief Default constructor.

   */
   GitQlientStyles() = default;
};
