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
