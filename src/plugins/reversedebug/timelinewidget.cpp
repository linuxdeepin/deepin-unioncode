/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "timelinewidget.h"
#include "event_man.h"
#include "reversedebuggerconstants.h"

#include <QPainter>
#include <QMenu>
#include <QTime>
#include <QDebug>
#include <QMouseEvent>

#include <assert.h>
#include <math.h>

#define SCALE_WIDTH         (100)
#define SCALE_HEIGHT        (10)
#define MIN_TIME_PER_SCALE  (1)
#define SCROLL_BAR_HEIGHT   (10)

namespace ReverseDebugger {
namespace Internal {

const int g_colors[] = {
    Qt::cyan,       // syscall
    Qt::red,        // signal
    Qt::blue,       // dbus
    Qt::magenta,    // x11
};

static QString formatTime(int curtime /*unit is ms*/)
{
    int h = 0;
    int m = 0;
    int s = 0;
    int ms = 0;

    // hh:mm:ss.zzz
    s = curtime/1000;
    ms = curtime - s*1000;
    m = s/60;
    s -= m*60;
    h = m/60;
    m -= h*60;

    return QString::asprintf("%02d:%02d:%02d.%03d", h, m, s, ms);
}

class TimelineWidgetPrivate
{
public:
    int visibleX = 0;
    int currentX = 0;
    int timePerScale = 50;   //unit is ms
    double firstTime = 0;
    double duration = 60.0 * 1000;   //unit is ms
    void *timeline = nullptr;
    int count = 0;
    int visibleBegin = 0;
    int visibleEnd = 0;
    int tid = -1;
    int eventBegin = -1;
    int eventEnd = -1;
    int eventIndexBegin = -1;
    int eventIndexEnd = -1;
    // bit 0:disable syscall
    // bit 1:disable signal;
    // bit 2:disable dbus;
    // bit 3:disable x11;
    int categoryIds = 0;

    QScrollBar *scroll = nullptr;
    TaskWindow *window = nullptr;

    QAction *zoomIn = nullptr;
    QAction *zoomOut = nullptr;
    QAction *zoomFit = nullptr;
};

TimelineWidget::TimelineWidget(QWidget *parent)
    : QWidget(parent),
      d(new TimelineWidgetPrivate())
{

    d->scroll = new QScrollBar(Qt::Horizontal, this);
    d->scroll->setRange(0, d->duration/d->timePerScale*SCALE_WIDTH);
    d->scroll->setSingleStep(SCALE_WIDTH);
    d->scroll->setMinimumHeight(SCROLL_BAR_HEIGHT);
    connect(d->scroll, &QScrollBar::valueChanged, this, &TimelineWidget::valueChanged);

    d->zoomIn = new QAction(tr("Zoom in"), this);
    connect(d->zoomIn, &QAction::triggered, this, &TimelineWidget::zoomIn);

    d->zoomOut = new QAction(tr("Zoom out"), this);
    connect(d->zoomOut, &QAction::triggered, this, &TimelineWidget::zoomOut);

    d->zoomFit = new QAction(tr("Fit view"), this);
    connect(d->zoomFit, &QAction::triggered, this, &TimelineWidget::zoomFit);
}

void TimelineWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(0, 0, width(), height(), QColor(0x40,0x42,0x44,255));

    int times = d->timePerScale;
    for (; times >= 10; times /= 10);

    // draw timeline
    // painter.setPen(Qt::black);
    painter.setPen(Qt::white);
    painter.setFont(QFont(QLatin1String("Arial"), SCALE_HEIGHT));
    painter.drawLine(0, SCALE_HEIGHT*2, width(), SCALE_HEIGHT*2);

    int step = SCALE_WIDTH/times;
    int begin = d->visibleX/SCALE_WIDTH*SCALE_WIDTH;
    for (int x = begin - d->visibleX, i = begin / SCALE_WIDTH * d->timePerScale;
            x < width();
            i += d->timePerScale) {
        QString val = formatTime(i);
        painter.drawText(x+2, SCALE_HEIGHT, val);

        painter.drawLine(x, SCALE_HEIGHT*2, x, SCALE_HEIGHT);

        for (int j = x + step; j < x + SCALE_WIDTH; j += step) {
            painter.drawLine(j, SCALE_HEIGHT*2, j, SCALE_HEIGHT*3/2);
        }
        x += SCALE_WIDTH;
    }

    // draw event graph
    int prev_x = -1;
    const EventEntry* entry = (d->timeline != nullptr) ?
        get_event_pointer(d->timeline) + d->visibleBegin : nullptr;

    int cur_color = Qt::black;

    for (int i = d->visibleBegin; i < d->visibleEnd; ++i, ++entry) {
        assert(entry->time >= d->firstTime);

        if ((d->categoryIds > 0) &&
            (d->categoryIds & (1 << (entry->type/1000 - __NR_Linux/1000)))) {
            // disable this category
            continue;
        }

        if (d->tid > 0 && entry->tid != d->tid)
            continue;

        if (d->eventIndexBegin >= 0 &&
            d->eventIndexEnd >= d->eventIndexBegin &&
            (i < d->eventIndexBegin || i > d->eventIndexEnd)){
            // not in index range!
            continue;
        }

        if (d->eventBegin >= 0 &&
            d->eventEnd >= d->eventBegin &&
            (entry->type < d->eventBegin || entry->type > d->eventEnd)){
            // not in event range!
            continue;
        }

        int x = (entry->time - d->firstTime)/d->timePerScale*SCALE_WIDTH - d->visibleX;
        if (x > prev_x) {
            int event_color = g_colors[entry->type/1000 - __NR_Linux/1000];
            if (event_color != cur_color) {
                painter.setPen(QColor(Qt::GlobalColor(event_color)));
                cur_color = event_color;
            }
            painter.drawLine(x, SCALE_HEIGHT*2 + 2, x, height() - SCROLL_BAR_HEIGHT);
            prev_x = x;
        }
        else if (Qt::darkYellow != cur_color) {
            // multiple events overlap
            cur_color = Qt::darkYellow;
            painter.setPen(QColor(Qt::GlobalColor(Qt::darkYellow)));
            painter.drawLine(prev_x, SCALE_HEIGHT*2 + 2,
                    prev_x, height() - SCROLL_BAR_HEIGHT);
        }
    }

    // TODO: how to update d->currentX if zoomed ?
    // draw current position if visible
    painter.setPen(Qt::yellow);
    if (d->currentX >= d->visibleX && d->currentX <= d->visibleX + width()) {
        int x = d->currentX - d->visibleX;
        int h = height() - SCROLL_BAR_HEIGHT;
        painter.drawLine(x, SCALE_HEIGHT*2, x, h);
    }
}

void TimelineWidget::mousePressEvent(QMouseEvent* event)
{
    if (Qt::LeftButton == event->button()) {
        d->currentX = event->x() + d->visibleX;
        update();
    }
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* event)
{
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
{
    // check if left button is pressed
    // d->currentX = event->x;
}

void TimelineWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    double x = event->x() + d->visibleX;
    double time = x/SCALE_WIDTH*d->timePerScale + d->firstTime;

    qDebug() << "double click at :" << x << ", " << time;

    // NOTE: index is relative to the full event list, not to the filter list;
    if (Qt::LeftButton == event->button() && d->timeline && d->window) {
        const EventEntry* entry = get_event_pointer(d->timeline) + d->visibleBegin;

        for (int i = d->visibleBegin; i<d->visibleEnd; ++i, ++entry) {
            if ((d->categoryIds > 0) &&
                (d->categoryIds & (1 << (entry->type/1000 - __NR_Linux/1000)))) {
                // disable this category
                continue;
            }

            if (d->eventIndexBegin >= 0 &&
                d->eventIndexEnd >= d->eventIndexBegin &&
                (i < d->eventIndexBegin || i > d->eventIndexEnd)){
                // not in index range!
                continue;
            }

            if (d->eventBegin >= 0 &&
                d->eventEnd >= d->eventBegin &&
                (entry->type < d->eventBegin || entry->type > d->eventEnd)){
                // not in event range!
                continue;
            }

            if (fabs(entry->time - time) < 0.05) {
                qDebug() << "double click event :" << i;
//                d->window->goTo(i); // mozart
                break;
            }
        }
    }
}

void TimelineWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    menu.addAction(d->zoomIn);
    menu.addAction(d->zoomOut);
    menu.addAction(d->zoomFit);
    menu.exec(event->globalPos());
}

void TimelineWidget::zoomIn()
{
    if (d->timePerScale <= MIN_TIME_PER_SCALE) {
        qDebug() << "reach minimum zoom level";
        return;
    }

    int scale = 1;
    int times = d->timePerScale;
    double cent_time = (d->visibleX + width()/2.0)/SCALE_WIDTH * d->timePerScale;
    for (; times >= 10; times /= 10, scale *= 10);
    if (5 == times) {
        d->timePerScale = 2 * scale;
    }
    else {
        d->timePerScale /= 2;
    }

    double cent_x = cent_time/d->timePerScale*SCALE_WIDTH;
    if (cent_x > width()/2.0) {
        d->visibleX = cent_x - width()/2.0;
    }
    else {
        d->visibleX = 0;
    }
    updateVisibleEvent();
    d->scroll->setValue(d->visibleX);
    d->scroll->setRange(0, d->duration/d->timePerScale*SCALE_WIDTH);
    update();

    qDebug() << "new unit:" << d->timePerScale << "ms, scroll range:" << d->scroll->maximum();
}

void TimelineWidget::zoomOut()
{
    int max = d->duration/d->timePerScale*SCALE_WIDTH;
    if (max <= width()) {
        qDebug() << "reach maximum zoom level " << max << "<=" << width();
        return;
    }

    int scale = 1;
    int times = d->timePerScale;
    double cent_time = (d->visibleX + width()/2.0)/SCALE_WIDTH * d->timePerScale;
    for (; times >= 10; times /= 10, scale *= 10);
    if (2 == times) {
        d->timePerScale = 5 * scale;
    }
    else {
        d->timePerScale *= 2;
    }

    double cent_x = cent_time/d->timePerScale*SCALE_WIDTH;
    if (cent_x > width()/2.0) {
        d->visibleX = cent_x - width()/2.0;
    }
    else {
        d->visibleX = 0;
    }
    updateVisibleEvent();
    d->scroll->setValue(d->visibleX);
    d->scroll->setRange(0, d->duration/d->timePerScale*SCALE_WIDTH);
    update();

    qDebug() << "new unit:" << d->timePerScale << "ms, scroll range:" << d->scroll->maximum();
}

void TimelineWidget::zoomFit()
{
    d->timePerScale = MIN_TIME_PER_SCALE;

    for (;;) {
        int max = d->duration/d->timePerScale*SCALE_WIDTH;
        qDebug() << "zoomFit try " << d->timePerScale
            << "ms, max:" << max << ", width:" << width();
        if (max < width() + 100) {
            break;
        }

        int scale = 1;
        int times = d->timePerScale;
        for (; times >= 10; times /= 10, scale *= 10);
        if (2 == times) {
            d->timePerScale = 5 * scale;
        }
        else {
            d->timePerScale *= 2;
        }
    }

    int max = d->duration/d->timePerScale*SCALE_WIDTH;
    d->visibleX = 0;
    updateVisibleEvent();
    d->scroll->setValue(0);
    d->scroll->setRange(0, max);
    d->scroll->setPageStep(max);
    update();

    qDebug() << "new unit:" << d->timePerScale << "ms, scroll range:" << d->scroll->maximum();
}

void TimelineWidget::resizeEvent(QResizeEvent *e)
{
    QSize size = e->size();
    d->scroll->setGeometry(0, size.height() - SCROLL_BAR_HEIGHT, size.width(), SCROLL_BAR_HEIGHT);
    d->scroll->setPageStep(size.width());
}

void TimelineWidget::valueChanged(int value)
{
    if (d->visibleX != value) {
        d->visibleX = value;
        updateVisibleEvent();
        update();
        qDebug() << "new pos:" << value;
    }
}

void TimelineWidget::setData(TaskWindow* window, void* timeline, int count)
{
    EventEntry entry;

    d->window = window;
    if (nullptr == timeline) {
        d->timeline = nullptr;
        d->count = 0;
        d->visibleBegin = 0;
        d->visibleEnd = 0;

        update();

        return;
    }

    d->timeline = timeline;
    d->count = count;
    get_event(d->timeline, 0, &entry);
    d->firstTime = entry.time;
    assert(d->firstTime > 0);
    get_event(d->timeline, count - 1, &entry);
    assert(entry.time > d->firstTime);
    d->duration = entry.time - d->firstTime;
    qDebug() << "set Duration:" << d->duration
        << ", first:" << d->firstTime << ", count:" << count;

    zoomFit();
}

void TimelineWidget::setEventTid(int tid)
{
    //NOTE: tid filter can combine with event type filter
    d->tid = tid;
    invalidateFilter();
}
void TimelineWidget::setEventRange(int begin, int end)
{
    d->eventBegin = begin;
    d->eventEnd = end;
    d->eventIndexBegin = -1;
    d->eventIndexEnd = -1;
    invalidateFilter();
}
void TimelineWidget::setEventIndexRange(int begin, int end)
{
    d->eventBegin = -1;
    d->eventEnd = -1;
    d->eventIndexBegin = begin;
    d->eventIndexEnd = end;
    invalidateFilter();
}

void TimelineWidget::updateVisibleEvent(void)
{
    if (!d->timeline) return;

    double begin_time = d->visibleX/SCALE_WIDTH*d->timePerScale;
    double end_time = (d->visibleX + width())/SCALE_WIDTH*d->timePerScale;

    // TODO: Should merge multiple event for best draw performance if
    // d->timePerScale >= 1s && d->count > 10K

    begin_time += d->firstTime;
    end_time += d->firstTime;

    const EventEntry* entry = get_event_pointer(d->timeline);
    for (int i = 0; i<d->count; ++i, ++entry) {
        if (entry->time >= begin_time) {
            d->visibleBegin = i;
            break;
        }
    }

    if (d->firstTime + d->duration < end_time) {
        d->visibleEnd = d->count;
        qDebug() << "visible event range:" << d->visibleBegin << "," << d->visibleEnd;
        return;
    }

    for (int i = d->visibleBegin + 1; i<d->count; ++i, ++entry) {
        if (entry->time > end_time) {
            d->visibleEnd = i;
            break;
        }
    }

    qDebug() << "visible event range:" << d->visibleBegin << "," << d->visibleEnd;
}

void TimelineWidget::setFilteredCategories(const QList<QString> &categoryIds)
{
    int mask = 0;
    QString ids[4] = {
        Constants::EVENT_CATEGORY_SYSCALL,
        Constants::EVENT_CATEGORY_SIGNAL,
        Constants::EVENT_CATEGORY_DBUS,
        Constants::EVENT_CATEGORY_X11,
    };

    for (int i = 0; i < categoryIds.size(); ++i) {
        if (categoryIds.at(i) == ids[0])
            mask |= 1;
        else if (categoryIds.at(i) == ids[1])
            mask |= 1<<1;
        else if (categoryIds.at(i) == ids[2])
            mask |= 1<<2;
        else if (categoryIds.at(i) == ids[3])
            mask |= 1<<3;
    }

    qDebug() << "d->categoryIds:" << (void*)mask;
    if (mask != d->categoryIds) {
        d->categoryIds = mask;
        invalidateFilter();
    }
}

void TimelineWidget::invalidateFilter()
{
    update();
}

} // namespace Internal
} // namespace ReverseDebugger

