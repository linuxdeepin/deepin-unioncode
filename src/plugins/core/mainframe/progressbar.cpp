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
#include "progressbar.h"

#include <QPaintEvent>
#include <QPainter>

class ProgressBarPrivate
{
    friend class ProgressBar;
    int percentageCache = 0;
    QColor defaultGreen = QColor(0, 0x99, 0);
    QColor brushColor = defaultGreen;
};

ProgressBar::ProgressBar(QWidget *parent)
    : QWidget(parent)
    , d(new ProgressBarPrivate)
{
    setObjectName("ProgressBar");
}

ProgressBar::~ProgressBar()
{
    if (d) {
        delete d;
    }
}

void ProgressBar::setPercentage(int percentage)
{
    d->percentageCache = percentage;
    setToolTip(QString::number(percentage) + "%");
}

void ProgressBar::setColor(const QColor &color)
{
    d->brushColor = color;
}

void ProgressBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::Text));
    QRect rect = this->rect().adjusted(0, 0, -1, -1);
    auto topLeft = rect.topLeft();
    auto topRight = rect.topRight();
    auto bottomLeft = rect.bottomLeft();
    auto bottomRight = rect.bottomRight();

    painter.drawLine(topLeft, topRight);
    painter.drawLine(topLeft, bottomLeft);
    painter.drawLine(topRight, bottomRight);
    painter.drawLine(bottomLeft, bottomRight);

    painter.setPen(Qt::NoPen);
    double rectWidth = rect.width();
    double drawWidth = rectWidth / 100 * d->percentageCache;
    QRect drawPercRect(1, 1, drawWidth, rect.height() - 2);
    painter.setBrush(d->brushColor);
    painter.drawRect(drawPercRect);
}
