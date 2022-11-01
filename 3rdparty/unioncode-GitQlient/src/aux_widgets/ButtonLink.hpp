/****************************************************************************************
 ** Copyright (C) 2016-2020  Francesc Martinez <es.linkedin.com/in/cescmm/en>
 **
 ** This file is part of DietPlanner.
 ** DietPlanner is an application to create, edit and manage diets, food and patients.
 **
 ** This application is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 3 of the License, or (at your option) any later version.
 **
 ** This application is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/
#ifndef BUTTONLINK_H
#define BUTTONLINK_H

#include <QLabel>
#include <QVariant>

/**
 * @brief This class creates a clickable QLabel that emits a signal when it's clicked and another one with the data the
 * user has configured
 *
 */
class ButtonLink : public QLabel
{
   Q_OBJECT

signals:
   /**
    * @brief Signal when the link has been clicked
    *
    */
   void clicked();

public:
   /**
    * @brief Default constructor of the ButtonLink class that creates an empty text button link.
    *
    * @param parent The parent widget
    */
   explicit ButtonLink(QWidget *parent = nullptr);

   /**
    * @brief Overload constructor of the ButtonLink class that creates a button link with text and data if the user sets
    * it.
    *
    * @param text The text that the will be shown
    * @param parent The parent widget
    */
   explicit ButtonLink(const QString &text, QWidget *parent = nullptr);

   /**
    * @brief Overload constructor of the ButtonLink class that creates a button link with text and data if the user sets
    * it.
    *
    * @param text The text that the will be shown
    * @param data The user data input
    * @param parent The parent widget
    */
   explicit ButtonLink(const QString &text, const QVariant &data, QWidget *parent = nullptr);

   void setData(const QVariant &data) { mData = data; }
   QVariant data() const { return mData; }

protected:
   /**
    *
    *@brief Event that processes whether the user  presses or not the mouse
    *
    * @param e The event
    */
   void mousePressEvent(QMouseEvent *e) override;

   /**
    * @brief Event that processes whether the user releases the mouse button or not
    *
    * @param event The event
    */
   void mouseReleaseEvent(QMouseEvent *event) override;

   /**
    * @brief Event that processes if the users enters the button link
    *
    * @param event The event
    */
   void enterEvent(QEvent *event) override;

   /**
    * @brief Event that processes if the users leaves the button link
    *
    * @param event The event
    */
   void leaveEvent(QEvent *event) override;

private:
   bool mPressed = false;
   QVariant mData;
};

#endif // BUTTONLINK_H
