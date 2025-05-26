// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentopenview.h"

#include <QKeyEvent>

RecentOpenView::RecentOpenView(QWidget *parent)
    : QTreeView(parent)
{
}

RecentOpenView::~RecentOpenView()
{
}

void RecentOpenView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace
                && event->modifiers() == 0) {
        emit closeActivated(currentIndex());
    } else {
        QTreeView::keyPressEvent(event);
    }
}
