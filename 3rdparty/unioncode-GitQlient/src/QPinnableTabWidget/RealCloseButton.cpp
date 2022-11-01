#include "RealCloseButton.h"

#include <QPainter>
#include <QStyleOption>

RealCloseButton::RealCloseButton(QWidget *parent)
   : QAbstractButton(parent)
{
   setFocusPolicy(Qt::NoFocus);
#ifndef QT_NO_CURSOR
   setCursor(Qt::ArrowCursor);
#endif
#ifndef QT_NO_TOOLTIP
   setToolTip(tr("Close Tab"));
#endif
   resize(sizeHint());
}

QSize RealCloseButton::sizeHint() const
{
   ensurePolished();
   int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, 0, this);
   int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, 0, this);
   return QSize(width, height);
}

void RealCloseButton::enterEvent(QEvent *event)
{
   if (isEnabled())
      update();
   QAbstractButton::enterEvent(event);
}

void RealCloseButton::leaveEvent(QEvent *event)
{
   if (isEnabled())
      update();
   QAbstractButton::leaveEvent(event);
}

void RealCloseButton::paintEvent(QPaintEvent *)
{
   QPainter p(this);
   QStyleOption opt;
   opt.init(this);
   opt.state |= QStyle::State_AutoRaise;
   if (isEnabled() && underMouse() && !isChecked() && !isDown())
      opt.state |= QStyle::State_Raised;
   if (isChecked())
      opt.state |= QStyle::State_On;
   if (isDown())
      opt.state |= QStyle::State_Sunken;

   if (const QTabBar *tb = qobject_cast<const QTabBar *>(parent()))
   {
      int index = tb->currentIndex();
      QTabBar::ButtonPosition position
          = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tb);
      if (tb->tabButton(index, position) == this)
         opt.state |= QStyle::State_Selected;
   }

   style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &p, this);
}
