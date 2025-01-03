// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "spinnerpainter.h"

#include <QtMath>
#include <QPainterPath>

SpinnerPainter::SpinnerPainter()
{
    refreshTimer.setInterval(30);
    QObject::connect(&refreshTimer, &QTimer::timeout, &refreshTimer,
                     [=]() {
                         currentDegree += 14;
                         callback();
                     });
}

void SpinnerPainter::paint(QPainter &painter, const QColor &color, const QRect &rect)
{
    painter.save();
    if (currentColor != color) {
        currentColor = color;
        indicatorColors.clear();
    }

    if (indicatorColors.isEmpty()) {
        for (int i = 0; i < 3; ++i)
            indicatorColors << createDefaultIndicatorColorList(color);
    }

    painter.setRenderHints(QPainter::Antialiasing);
    auto center = QRectF(rect).center();
    auto radius = qMin(rect.width(), rect.height()) / 2.0;
    auto indicatorRadius = radius / 2 / 2 * 1.1;
    auto indicatorDegreeDelta = 360 / indicatorColors.count();

    for (int i = 0; i < indicatorColors.count(); ++i) {
        auto colors = indicatorColors.value(i);
        for (int j = 0; j < colors.count(); ++j) {
            double degreeCurrent = currentDegree - j * indicatorShadowOffset + indicatorDegreeDelta * i;
            auto x = (radius - indicatorRadius) * qCos(qDegreesToRadians(degreeCurrent));
            auto y = (radius - indicatorRadius) * qSin(qDegreesToRadians(degreeCurrent));

            x = center.x() + x;
            y = center.y() + y;
            auto tl = QPointF(x - 1 * indicatorRadius, y - 1 * indicatorRadius);
            QRectF rf(tl.x(), tl.y(), indicatorRadius * 2, indicatorRadius * 2);

            QPainterPath path;
            path.addEllipse(rf);

            painter.fillPath(path, colors.value(j));
        }
    }
    painter.restore();
}

void SpinnerPainter::setUpdateCallback(const UpdateCallback &cb)
{
    callback = cb;
}

void SpinnerPainter::startAnimation()
{
    refreshTimer.start();
}

void SpinnerPainter::stopAnimation()
{
    refreshTimer.stop();
}

QList<QColor> SpinnerPainter::createDefaultIndicatorColorList(QColor color)
{
    QList<QColor> colors;
    QList<int> opacitys;
    opacitys << 100 << 30 << 15 << 10 << 5 << 4 << 3 << 2 << 1;
    for (int i = 0; i < opacitys.count(); ++i) {
        color.setAlpha(255 * opacitys.value(i) / 100);
        colors << color;
    }
    return colors;
}
