#include "detailsbutton.h"

#include <QPainter>
#include <QStyleOption>
#include <QPalette>
#include <QDebug>
#include <QPaintEvent>
#include <QPropertyAnimation>

class DetailsButtonPrivate
{
    friend class DetailsButton;
    bool hover = false;
};

DetailsButton::DetailsButton(QWidget *parent)
    : QAbstractButton (parent)
    , d(new DetailsButtonPrivate)
{
    setCheckable(true);
    setText(tr("Details"));
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

DetailsButton::~DetailsButton()
{
    if (d)
        delete d;
}

void DetailsButton::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    int arrowsize = 15;

    if (!d->hover) {
        p.save();
        p.setPen(Qt::NoPen);
        p.setBrush(palette().brush(QPalette::Light));
        p.drawRoundedRect(rect(), 1, 1);
        p.restore();
    }

    QFont font(text());
    QFontMetrics fontMetrics(this->font());
    QRect textBoundingRect = fontMetrics.boundingRect(text());
    QSize fontSize = textBoundingRect.size();
    int fontPosX = textBoundingRect.x();
    int fontPosY = textBoundingRect.y();
    int textPosX = (size().width() - fontSize.width() - arrowsize) / 2
            + ( fontPosX > 0 ? fontPosX : - fontPosX);
    int textPosY = (size().height() - fontSize.height()) / 2
            + ( fontPosY > 0 ? fontPosY: - fontPosY);
    p.drawText(QPoint{textPosX, textPosY}, this->text());

    bool checked = isChecked();
    QStyleOption arrowOpt;
    arrowOpt.initFrom(this);
    QPalette pal = arrowOpt.palette;
    pal.setBrush(QPalette::All, QPalette::Text, QColor(0, 0, 0));
    arrowOpt.rect = QRect{size().width() - arrowsize - 6, (height() - arrowsize) / 2  , arrowsize, arrowsize};
    arrowOpt.palette = pal;
    style()->drawPrimitive(checked ? QStyle::PE_IndicatorArrowUp : QStyle::PE_IndicatorArrowDown, &arrowOpt, &p, this);

    if (d->hover) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 20));
        p.drawRoundedRect(rect(), 1, 1 );
    }
}

void DetailsButton::enterEvent(QEvent *event)
{
    d->hover = true;
    return QAbstractButton::enterEvent(event);
}

void DetailsButton::leaveEvent(QEvent *event)
{
    d->hover = false;
    return QAbstractButton::enterEvent(event);
}
