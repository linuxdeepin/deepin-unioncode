// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <DWidget>
#include <QTimer>
#include <QPainter>
#include <QDebug>
#include <QtMath>
class StatusWidgetPrivate;
class StatusWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
    StatusWidgetPrivate *const d;
public:
    //time Control speed from mes
    enum  RotaSpeed
    {
        Rapidly = 50,
        Fast = 65,
        Usual = 80,
        Slow = 100,
        Turtle = 120,
    };

    enum PatternFlag
    {
        Simple = 0x01,
        Radar = 0x02,
        Ring = 0x04,
        Twinkle = 0x08,
        Ripple = 0x0F
    };

    Q_DECLARE_FLAGS(PatternFlags, PatternFlag)

    // Radar = Simple|Radar
    // Ring = Simple|Ring

    enum State
    {
        Running,
        Error,
        Stoped,
    };

    explicit StatusWidget(QWidget *parent = nullptr);
    StatusWidget(PatternFlags flags, QWidget *parent = nullptr);
    virtual ~StatusWidget();
    void setElemCount(int count);
    int elemCount();
    void setPatternFlags(StatusWidget::PatternFlags flags);
    StatusWidget::PatternFlags patternFlags();
    void setRotaSpeed(StatusWidget::RotaSpeed speed);
    RotaSpeed rotaSpeed() const;
    //timer start
    void start(StatusWidget::RotaSpeed speed);
    void start();
    void stop();
    //get timer and flag state
    bool isRunning();
    bool isStoped();
    //paint error
    void setError();
    bool isError();
    //paint warning
    void setWarning();
    bool isWarning();
    //extend method and to setting run state color
    void setRunningColor(QColor color);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void paintError(QPainter &painter);
    virtual void paintSimple(QPainter &painter);
    virtual void paintRadar(QPainter &painter);
    virtual void paintRing(QPainter &painter);
    virtual void paintTwinkle(QPainter &painter);
    virtual void paintRipple(QPainter &painter);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StatusWidget::PatternFlags)

#endif // STATUSWIDGET_H
