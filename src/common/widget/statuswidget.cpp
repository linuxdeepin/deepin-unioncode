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
#include "statuswidget.h"
#include <QPaintEvent>
#include <QResizeEvent>

class StatusWidgetPrivate
{
    friend class StatusWidget;
    QRectF drawRange;
    QTimer timer;
    QColor color = Qt::green;
    StatusWidget::PatternFlags flags = StatusWidget::PatternFlag::Simple;
    int currElemIndex = -1;
    int elem = 8;
    StatusWidget::State state = StatusWidget::State::Stoped;
};

StatusWidget::StatusWidget(QWidget *parent)
    : QWidget (parent)
    , d (new StatusWidgetPrivate)
{

}

StatusWidget::StatusWidget(PatternFlags flags, QWidget *parent)
    : StatusWidget(parent)
{
    d->flags = flags;
}

StatusWidget::~StatusWidget()
{
    if (d) {
        delete d;
    }
}

void StatusWidget::setElemCount(int count)
{
    d->elem = count;
}

int StatusWidget::elemCount()
{
    return d->elem;
}

void StatusWidget::setPatternFlags(StatusWidget::PatternFlags flags)
{
    d->flags = flags;
}

StatusWidget::PatternFlags StatusWidget::patternFlags()
{
    return d->flags;
}

void StatusWidget::start(StatusWidget::RotaSpeed speed){
    QObject::connect(&d->timer,&QTimer::timeout,this,[=](){
        this->update();
    });
    d->timer.start(speed);
    d->state = Running;
}

void StatusWidget::setRotaSpeed(StatusWidget::RotaSpeed speed)
{
    if(d->timer.interval() > 0)
        d->timer.start(speed);
}

void StatusWidget::start()
{
    this->start(RotaSpeed::Usual);
}

StatusWidget::RotaSpeed StatusWidget::rotaSpeed() const
{
    return static_cast<RotaSpeed>(d->timer.interval());
}

void StatusWidget::stop()
{
    d->timer.stop();
    d->state = Stoped;
}

bool StatusWidget::isRunning()
{
    if(d->timer.isActive() != (d->state == State::Running))
    {
        qWarning() << "unknown errorr from timer paint";
    }

    return d->timer.isActive() && d->state == State::Running;
}

bool StatusWidget::isStoped()
{
    return (!d->timer.isActive()) && d->state == State::Stoped;
}

void StatusWidget::setError()
{
    d->timer.stop();
    d->state = Error;
}

bool StatusWidget::isError()
{
    return d->state == State::Error;
}

void StatusWidget::setWarning()
{
    d->color = Qt::yellow;
}

bool StatusWidget::isWarning()
{
    return d->color == Qt::yellow;
}

void StatusWidget::setRunningColor(QColor color)
{
    d->color = color;
}

void StatusWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if(d->state == State::Stoped) return;

    QPainter painter(this);

    if(d->drawRange.isEmpty() || d->drawRange != this->rect())
        d->drawRange = this->rect();

    if(d->elem <= 0)
        d->elem = 8;

    if(d->state == State::Error)
        return paintError(painter);

    if(d->flags.testFlag(PatternFlag::Ripple))
        return paintRipple(painter);

    if(d->flags.testFlag(PatternFlag::Twinkle))
        return paintTwinkle(painter);

    if(d->flags == PatternFlag::Ring
            || d->flags.testFlag(PatternFlag::Simple))
        paintSimple(painter);

    if(d->flags.testFlag(PatternFlag::Radar))
        paintRadar(painter);
}

void StatusWidget::paintError(QPainter &painter)
{
    QRadialGradient gradient(this->rect().center(),
                             qMax(this->width(),this->height())/2,
                             this->rect().center());

    gradient.setColorAt(0, Qt::red);
    gradient.setColorAt(1, palette().background().color());
    gradient.setSpread(QGradient::ReflectSpread);
    painter.setPen(Qt::red);
    painter.setBrush(QBrush(gradient));
    auto len = qMin(this->width(), this->height()) / 3;
    painter.drawEllipse(this->rect().center(), len,len);
}

void StatusWidget::paintSimple(QPainter &painter)
{
    qreal angle = (360 / d->elem) *16;

    if(d->currElemIndex == -1 || d->currElemIndex >= d->elem)
        d->currElemIndex = 0;

    //    QRadialGradient gradient(this->rect().center(),
    //                             qMax(this->width(),this->height())/2,
    //                             this->rect().center());
    //    gradient.setColorAt(0, _color);
    //    gradient.setColorAt(1,palette().background().color());
    //    gradient.setSpread(QGradient::ReflectSpread);
    painter.setPen(palette().background().color());
    //    painter.setBrush(QBrush(gradient));
    painter.setBrush(QBrush(d->color));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPie(d->drawRange, d->currElemIndex * angle, angle);
    d->currElemIndex ++;

    if (d->flags.testFlag(PatternFlag::Ring)) paintRing(painter);

}

void StatusWidget::paintRadar(QPainter &painter)
{
    qreal angle = 360 / d->elem;

    static int currPieIndex = -1;
    if(currPieIndex == -1 || currPieIndex >= d->elem)
        currPieIndex = 0;

    QConicalGradient gradient(this->rect().center(),
                              (currPieIndex+1) * angle);
    gradient.setColorAt(0, palette().background().color());
    gradient.setColorAt(1, d->color);
    gradient.setSpread(QGradient::ReflectSpread);
    painter.setPen(palette().background().color());
    painter.setBrush(QBrush(gradient));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawEllipse(this->rect().center(),this->width()/2,this->height()/2);
    currPieIndex ++;

    if (d->flags.testFlag(PatternFlag::Ring)) paintRing(painter);
}

void StatusWidget::paintRing(QPainter &painter)
{
    auto background = this->palette().color(this->backgroundRole());
    painter.setPen(background);
    painter.setBrush(background);
    painter.drawEllipse(this->rect().center(), this->width() / 3,this->height() / 3);
}

void StatusWidget::paintTwinkle(QPainter &painter)
{
    auto background = this->palette().background();
    painter.setPen(background.color());

    if(d->currElemIndex % 2 == 0)
    {
        painter.setBrush(this->palette().background());
    }
    else
    {
        painter.setBrush(QColor(d->color));
    }

    painter.drawEllipse(this->rect().center(),this->width()/3,this->height()/3);
    d->currElemIndex ++;
}

void StatusWidget::paintRipple(QPainter &painter)
{
    Q_UNUSED(painter);
}
