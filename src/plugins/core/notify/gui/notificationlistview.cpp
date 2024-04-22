// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationlistview.h"

#include <QScroller>

DWIDGET_USE_NAMESPACE

NotificationListView::NotificationListView(QWidget *parent)
    : DListView(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(this);
    QScrollerProperties sp;
    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    scroller->setScrollerProperties(sp);
}

NotificationListView::~NotificationListView()
{

}
