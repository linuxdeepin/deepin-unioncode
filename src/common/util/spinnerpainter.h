// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SPINNERPAINTER_H
#define SPINNERPAINTER_H

#include <QPainter>
#include <QTimer>

#include <functional>

class SpinnerPainter
{
public:
    SpinnerPainter();

    void paint(QPainter &painter, const QColor &color, const QRect &rect);

    using UpdateCallback = std::function<void()>;
    void setUpdateCallback(const UpdateCallback &cb);

    void startAnimation();
    void stopAnimation();

protected:
    QList<QColor> createDefaultIndicatorColorList(QColor color);

private:
    QTimer refreshTimer;

    double indicatorShadowOffset = 10;
    double currentDegree = 0.0;

    QList<QList<QColor>> indicatorColors;
    QColor currentColor;
    UpdateCallback callback;
};

#endif   // SPINNERPAINTER_H
