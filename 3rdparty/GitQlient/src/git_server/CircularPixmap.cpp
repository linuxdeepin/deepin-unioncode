#include "CircularPixmap.h"

#include <QPainter>
#include <QPainterPath>

CircularPixmap::CircularPixmap(const QSize &size, QWidget *parent)
   : QLabel(parent)
   , mSize(size)
{
}

CircularPixmap::CircularPixmap(const QString &filePath, QWidget *parent)
   : QLabel(parent)
   , mSize(50, 50)
{
   QPixmap px(filePath);
   px = px.scaled(mSize.width(), mSize.height());

   setPixmap(filePath);
   setFixedSize(mSize);
}
#include <QPaintEvent>
void CircularPixmap::paintEvent(QPaintEvent *e)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
   if (pixmap())
#endif
   {
      const auto rect = e->rect();
      const auto startX = (rect.width() - mSize.width()) / 2;
      const auto startY = mCenterPosition ? (rect.height() - mSize.height()) / 2 : 0;

      QPainter painter(this);
      painter.setRenderHint(QPainter::Antialiasing);

      QPainterPath path;
      path.addEllipse(startX, startY, mSize.width(), mSize.height());
      painter.setClipPath(path);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
      painter.drawPixmap(startX, 0, mSize.width(), mSize.height(), *pixmap());
#else
      painter.drawPixmap(startX, startY, mSize.width(), mSize.height(), pixmap(Qt::ReturnByValue));
#endif
   }
}
