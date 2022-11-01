#include <CheckBox.h>

#include <QEvent>
#include <QStyleOptionButton>
#include <QStylePainter>

#include <array>

namespace
{
static std::array<const char *, 6> indicators { ":/icons/qcb",   ":/icons/qcb_c",   ":/icons/qcb_i",
                                                ":/icons/qcb_d", ":/icons/qcb_d_c", ":/icons/qcb_d_i" };
}

CheckBox::CheckBox(QWidget *parent)
   : QCheckBox(parent)
{
}

CheckBox::CheckBox(const QString &text, QWidget *parent)
   : QCheckBox(text, parent)
{
}

QString CheckBox::getIndicator(QStyle::State state) const
{
   if (state & QStyle::State_Off)
      return QString::fromUtf8((state & QStyle::State_Enabled) ? indicators[0] : indicators[3]);
   else if (state & QStyle::State_On)
      return QString::fromUtf8((state & QStyle::State_Enabled) ? indicators[1] : indicators[4]);
   else if (state & QStyle::State_NoChange)
      return QString::fromUtf8((state & QStyle::State_Enabled) ? indicators[2] : indicators[5]);

   return QString();
}

bool CheckBox::event(QEvent *e)
{
   if (e->type() == QEvent::Wheel)
      return false;

   return QCheckBox::event(e);
}

void CheckBox::paintEvent(QPaintEvent *)
{
   QStylePainter painter(this);

   QStyleOptionButton option;

   initStyleOption(&option);

   painter.drawControl(QStyle::CE_CheckBox, option);

   const QRect rect = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, this);

   painter.drawPixmap(rect, QIcon(getIndicator(option.state)).pixmap(rect.size()));
}
