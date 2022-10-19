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
#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QObject>
#include <QEvent>
#include <QAction>
#include <QScrollBar>

namespace ReverseDebugger{
namespace Internal {

class TaskWindow;
class TimelineWidgetPrivate;
class TimelineWidget : public QWidget
{
public:
    TimelineWidget(QWidget *parent);
    void setData(TaskWindow* window, void* timeline, int count);
    void setEventTid(int tid);
    void setEventRange(int begin, int end);
    void setEventIndexRange(int begin, int end);
    void setFilteredCategories(const QList<QString> &categoryIds);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomFit();
    void valueChanged(int value);

private:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void contextMenuEvent(QContextMenuEvent*);
    void resizeEvent(QResizeEvent *e);
    void updateVisibleEvent(void);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void invalidateFilter();

    TimelineWidgetPrivate *const d;
};

} // namespace Internal
} // namespace ReverseDebugger

#endif // TIMELINEWIDGET_H
