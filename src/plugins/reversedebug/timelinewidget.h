// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
