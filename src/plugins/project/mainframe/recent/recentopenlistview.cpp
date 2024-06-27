// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentopenlistview.h"

#include <QKeyEvent>

RecentOpenListView::RecentOpenListView(QWidget *parent)
    : DListView(parent)
{
}

RecentOpenListView::~RecentOpenListView()
{
}

void RecentOpenListView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace
                && event->modifiers() == 0) {
        emit closeActivated(currentIndex());
    } else {
        DListView::keyPressEvent(event); 
    }
}
