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

#include <QMap>
#include <QTabWidget>

class QPinnableTabWidget : public QTabWidget
{
   Q_OBJECT

public:
   explicit QPinnableTabWidget(QWidget *parent = nullptr);
   ~QPinnableTabWidget() override = default;

   int addPinnedTab(QWidget *page, const QString &label);
   int addPinnedTab(QWidget *page, const QIcon &icon, const QString &label);

   int addTab(QWidget *widget, const QString &s);
   int addTab(QWidget *widget, const QIcon &icon, const QString &label);

   int insertTab(int index, QWidget *widget, const QString &s);
   int insertTab(int index, QWidget *widget, const QIcon &icon, const QString &label);

   void removeTab(int index);

   bool tabsClosable() const { return QTabWidget::tabsClosable(); }

   void clear();

   bool isPinned(int index);

   int getLastPinnedTabIndex() const;

protected:
   void mouseReleaseEvent(QMouseEvent *event) override;

private:
   int mLastPinnedTab = -1;
   QMap<int, bool> mTabState;
   bool mPrepareMenu = false;
   int mClickedTab = -1;
   int mLastPinTab = 0;

   void clickRequested(int index);
   void showContextMenu();
   void pinTab();
   void unpinTab();
   QTabBar *tabBar() const { return QTabWidget::tabBar(); }
   void setTabsClosable(bool closeable) { QTabWidget::setTabsClosable(closeable); }

   int indexAtPos(const QPoint &p);
};
