#include "elidedlabel.h"

#include <QString>

class ElidedLabelPrivate
{
    friend class ElidedLabel;
    QString sourceText;
};

ElidedLabel::ElidedLabel(QWidget *parent)
    : QLabel (parent)
    , d (new ElidedLabelPrivate)
{

}

ElidedLabel::~ElidedLabel()
{
    if (d) {
        delete d;
    }
}

void ElidedLabel::setText(const QString &text)
{
    d->sourceText = text;
    QString resultText;
    QFontMetrics font(this->font());
    int font_size = font.width(text);
    int resize_width = width();
    if(font_size > resize_width) {
        resultText = font.elidedText(d->sourceText, Qt::ElideRight, resize_width);
    } else {
        resultText = d->sourceText;
    }
    QLabel::setText(resultText);
    QLabel::setToolTip(text);
}

QString ElidedLabel::text()
{
    return d->sourceText;
}
