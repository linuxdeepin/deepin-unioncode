// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <DWidget>
#include <DScrollBar>

#include <QObject>
#include <QObject>
#include <QEvent>
#include <QAction>

namespace ReverseDebugger {
namespace Internal {

class TaskWindow;
class TimelineWidgetPrivate;
class TimelineWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
public:
    explicit TimelineWidget(QWidget *parent);
    ~TimelineWidget() override;

    void setData(TaskWindow *window, void *timeline, int count);
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
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    void resizeEvent(QResizeEvent *e) override;
    void updateVisibleEvent(void);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void invalidateFilter();

    TimelineWidgetPrivate *const d;
};

}   // namespace Internal
}   // namespace ReverseDebugger

#endif   // TIMELINEWIDGET_H
