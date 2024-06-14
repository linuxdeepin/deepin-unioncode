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
    if (event->key() == Qt::Key_Tab) {
        QModelIndex currentIndex = this->currentIndex();  
        if (currentIndex.isValid()) {  
            QModelIndex nextIndex = model()->index(currentIndex.row() + 1, currentIndex.column(), currentIndex.parent());  
            if (nextIndex.isValid()) {  
                setCurrentIndex(nextIndex);  
            } else {  
                setCurrentIndex(model()->index(0, 0));
            }  
        }
    } else {
        DListView::keyPressEvent(event); 
    }
}
