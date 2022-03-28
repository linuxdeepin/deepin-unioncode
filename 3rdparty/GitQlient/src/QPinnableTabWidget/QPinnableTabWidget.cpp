#include "QPinnableTabWidget.h"

#include "FakeCloseButton.h"
#include "RealCloseButton.h"

#include <QMenu>
#include <QMouseEvent>
#include <QTabBar>
#include <QStyle>

namespace
{
class TabBarPin : public QTabBar
{
public:
   explicit TabBarPin(QWidget *parent = nullptr)
      : QTabBar(parent)
   {
   }

protected:
   void mousePressEvent(QMouseEvent *event) override
   {
      mIndexToMove = indexAtPos(event->pos());
      mDistToStart = event->x() - tabRect(mIndexToMove).x();

      QTabBar::mousePressEvent(event);
   }

   void mouseMoveEvent(QMouseEvent *event) override
   {
      const auto pinnableWidget = dynamic_cast<QPinnableTabWidget *>(parentWidget());

      if (pinnableWidget)
      {
         const auto currentPinned = pinnableWidget->isPinned(mIndexToMove);
         const auto newPosIsPinned = pinnableWidget->isPinned(indexAtPos(event->pos()));
         const auto lastPinnedTab = pinnableWidget->getLastPinnedTabIndex();

         if (!currentPinned && !newPosIsPinned && (event->pos().x() - mDistToStart) > tabRect(lastPinnedTab).right())
            QTabBar::mouseMoveEvent(event);
      }
   }

   void mouseReleaseEvent(QMouseEvent *event) override
   {
      mIndexToMove = -1;
      mDistToStart = 0;

      QTabBar::mouseReleaseEvent(event);
   }

private:
   int mIndexToMove = -1;
   int mDistToStart = 0;

   int indexAtPos(const QPoint &p)
   {
      if (tabRect(currentIndex()).contains(p))
         return currentIndex();

      for (int i = 0; i < count(); ++i)
         if (isTabEnabled(i) && tabRect(i).contains(p))
            return i;

      return -1;
   }
};
}

QPinnableTabWidget::QPinnableTabWidget(QWidget *parent)
   : QTabWidget(parent)
{
   setTabBar(new TabBarPin());

   setTabsClosable(true);
   setMovable(true);
   connect(this, &QTabWidget::tabCloseRequested, this, &QPinnableTabWidget::removeTab, Qt::QueuedConnection);
   connect(this, &QTabWidget::tabBarClicked, this, &QPinnableTabWidget::clickRequested);
}

int QPinnableTabWidget::addPinnedTab(QWidget *page, const QString &label)
{
   const auto tabIndex = addTab(page, label);
   tabBar()->setTabButton(
       tabIndex,
       static_cast<QTabBar::ButtonPosition>(style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this)),
       new FakeCloseButton());

   tabBar()->moveTab(tabIndex, mLastPinTab);

   mTabState.insert(mLastPinTab++, true);

   return tabIndex;
}

int QPinnableTabWidget::addPinnedTab(QWidget *page, const QIcon &icon, const QString &label)
{
   const auto tabIndex = addTab(page, icon, label);
   tabBar()->setTabButton(
       tabIndex,
       static_cast<QTabBar::ButtonPosition>(style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this)),
       new FakeCloseButton());

   tabBar()->moveTab(tabIndex, mLastPinTab);

   mTabState.insert(mLastPinTab++, true);

   return tabIndex;
}

int QPinnableTabWidget::addTab(QWidget *widget, const QString &s)
{
   return QTabWidget::addTab(widget, s);
}

int QPinnableTabWidget::addTab(QWidget *widget, const QIcon &icon, const QString &label)
{
   return QTabWidget::addTab(widget, icon, label);
}

int QPinnableTabWidget::insertTab(int index, QWidget *widget, const QString &s)
{
   if (index <= mLastPinTab)
      index = mLastPinTab + 1;

   return QTabWidget::insertTab(index, widget, s);
}

int QPinnableTabWidget::insertTab(int index, QWidget *widget, const QIcon &icon, const QString &label)
{
   if (index <= mLastPinTab)
      index = mLastPinTab + 1;

   return QTabWidget::insertTab(index, widget, icon, label);
}

void QPinnableTabWidget::removeTab(int index)
{
   QTabWidget::removeTab(index);

   if (mTabState.value(index))
      --mLastPinTab;

   mTabState.remove(index);
}

void QPinnableTabWidget::clear()
{
   QTabWidget::clear();
   mLastPinnedTab = -1;
   mTabState.clear();
   mPrepareMenu = false;
   mClickedTab = -1;
   mLastPinTab = 0;
}

bool QPinnableTabWidget::isPinned(int index)
{
   return mTabState.contains(index);
}

int QPinnableTabWidget::getLastPinnedTabIndex() const
{
   return mLastPinTab - 1;
}

void QPinnableTabWidget::mouseReleaseEvent(QMouseEvent *event)
{
   if (event->button() == Qt::RightButton)
      showContextMenu();
   else
      mClickedTab = -1;
}

void QPinnableTabWidget::clickRequested(int index)
{
   mPrepareMenu = true;
   mClickedTab = index;
}

void QPinnableTabWidget::showContextMenu()
{
   if (!mPrepareMenu)
      return;

   const auto actions = new QMenu(this);

   if (mTabState.value(mClickedTab))
      connect(actions->addAction("Unpin"), &QAction::triggered, this, &QPinnableTabWidget::unpinTab);
   else
      connect(actions->addAction("Pin"), &QAction::triggered, this, &QPinnableTabWidget::pinTab);

   connect(actions->addAction("Close"), &QAction::triggered, this, [this]() { emit tabCloseRequested(mClickedTab); });

   actions->exec(QCursor::pos());
}

void QPinnableTabWidget::pinTab()
{
   tabBar()->setTabButton(
       mClickedTab,
       static_cast<QTabBar::ButtonPosition>(style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this)),
       new FakeCloseButton());

   if (mClickedTab != mLastPinTab)
      tabBar()->moveTab(mClickedTab, mLastPinTab);

   mTabState.insert(mLastPinTab, true);

   mLastPinTab = mTabState.count();

   mClickedTab = -1;
}

void QPinnableTabWidget::unpinTab()
{
   const auto closeBtn = new RealCloseButton();

   tabBar()->setTabButton(
       mClickedTab,
       static_cast<QTabBar::ButtonPosition>(style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this)),
       closeBtn);

   mTabState.remove(mClickedTab);

   mLastPinTab = mTabState.count();

   auto deletions = false;

   for (auto pair : mTabState.toStdMap())
   {
      if (pair.first > mClickedTab)
      {
         mTabState[pair.first - 1] = pair.second;
         deletions = true;
      }
   }

   if (deletions)
      mTabState.remove(mTabState.lastKey());

   tabBar()->moveTab(mClickedTab, mLastPinTab);

   connect(closeBtn, &RealCloseButton::clicked, this, [this]() { emit tabBar()->tabCloseRequested(mLastPinTab); });

   mClickedTab = -1;
}
