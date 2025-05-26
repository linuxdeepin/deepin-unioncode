// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "progressbar.h"

#include <QPaintEvent>
#include <QPainter>

DWIDGET_USE_NAMESPACE

class ProgressBarPrivate
{
    friend class ProgressBar;
    int percentageCache = 0;
    QColor defaultGreen = QColor(0, 0x99, 0);
    QColor brushColor = defaultGreen;
};

ProgressBar::ProgressBar(QWidget *parent)
    : DWidget(parent)
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
