#include "FakeCloseButton.h"

FakeCloseButton::FakeCloseButton(QWidget *parent)
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
