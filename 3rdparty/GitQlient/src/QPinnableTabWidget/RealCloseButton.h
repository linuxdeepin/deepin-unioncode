#pragma once

/****************************************************************************************
 ** QPinnableTabWidget is a library that overrides a QTabWidget to allow the user to pin tabs.
 **
 ** Copyright (C) 2021  Francesc Martinez
 **
 ** LinkedIn: www.linkedin.com/in/cescmm/
 ** Web: www.francescmm.com
 **
 ** This library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License as published by the Free Software Foundation; either
 ** version 2 of the License, or (at your option) any later version.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ** Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this library; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ***************************************************************************************/

#include <QAbstractButton>

class RealCloseButton : public QAbstractButton
{
   Q_OBJECT

public:
   explicit RealCloseButton(QWidget *parent = 0);

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override { return sizeHint(); }
   void enterEvent(QEvent *event) override;
   void leaveEvent(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
};
