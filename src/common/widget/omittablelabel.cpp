#include "omittablelabel.h"

#include <QPaintEvent>
#include <QPainter>

class OmittableLabelPrivate
{
    friend class OmittableLabel;
    Qt::TextElideMode mode = Qt::ElideRight;
    QString text = "";
};

OmittableLabel::OmittableLabel(QWidget * parent)
    : QLabel (parent)
    , d(new OmittableLabelPrivate)
{
    setWordWrap(false);
}

OmittableLabel::~OmittableLabel()
{
    if (d)
        delete d;
}

void OmittableLabel::setTextElideMode(Qt::TextElideMode mode)
{
    d->mode = mode;
}

void OmittableLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QFontMetrics fontMetrics(font());
    QLabel::setText(fontMetrics.elidedText(d->text, d->mode, width()));
}

void OmittableLabel::setText(const QString &text)
{
    d->text = text;
}

QString OmittableLabel::text()
{
    return d->text;
}
