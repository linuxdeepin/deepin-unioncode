/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
    int font_size = font.horizontalAdvance(text);
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
