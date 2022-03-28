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

class QLabel;

/**
 * @brief The ClickableFrame class is simple widget to make QLabel clickable and at the same time give the ability to
 * customize its look & feel. The behaviour is so simplified that only has a @p clicked() signal to notify the user that
 * the object was clicked.
 *
 * Another reason to have the widget is that it allows the user to fill it with other widgets keeping the click
 * behaviour in top of them.
 *
 * @class ClickableFrame ClickableFrame.h "ClickableFrame.h"
 */
class ClickableFrame : public QFrame
{
   Q_OBJECT

signals:
   /**
    * @brief Signal emitted when the user clicks the object.
    */
   void clicked();

public:
   /**
    * @brief Default constructor. This creates an empty
    *
    * @param parent The parent widget if needed.
    */
   explicit ClickableFrame(QWidget *parent = nullptr);
   /**
    * @brief Creates a ClickableFrame with a QLabel inside filled with the given @p text and in the given @p alignment
    *
    * @param text The text that the widget will display
    * @param alignment The alignment of the text in the widget
    * @param parent The parent widget if needed
    */
   explicit ClickableFrame(const QString &text, Qt::Alignment alignment, QWidget *parent = nullptr);

   /**
    * @brief setLinkStyle Sets the text of the widget overlined when hover.
    */
   void setLinkStyle() { mHasLinkStyles = true; }

protected:
   /**
    * @brief Detects the press event to prepare the click signal.
    *
    * @param e The event
    */
   void mousePressEvent(QMouseEvent *e) override;
   /**
    * @brief Detects the release event and if the press was detected before, it triggers the clicked signal.
    *
    * @param e The event
    */
   void mouseReleaseEvent(QMouseEvent *e) override;

   /**
    * @brief enterEvent Detects the enter event and in case the link style is enabled it applies it.
    * @param event The event.
    */
   void enterEvent(QEvent *event) override;

   /**
    * @brief leaveEvent Detects the leave event and in case the link style is enabled it removes the underline.
    * @param event
    */
   void leaveEvent(QEvent *event) override;

private:
   bool mPressed = false;
   bool mHasLinkStyles = false;
   QLabel *mText = nullptr;
};
