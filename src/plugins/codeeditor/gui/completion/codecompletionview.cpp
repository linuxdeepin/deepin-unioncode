// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codecompletionview.h"
#include "codecompletiondelegate.h"

CodeCompletionView::CodeCompletionView(QWidget *parent)
    : QListView(parent)
{
    setItemDelegate(new CodeCompletionDelegate(this));
    setIconSize(QSize(16, 16));
}

void CodeCompletionView::nextCompletion()
{
    QModelIndex firstCurrent = model()->index(0, 0);
    QModelIndex oldCurrent = currentIndex();
    QModelIndex current = moveCursor(MoveDown, Qt::NoModifier);

    if (current != oldCurrent && current.isValid()) {
        setCurrentIndex(current);
    } else {
        if (firstCurrent.isValid())
            setCurrentIndex(firstCurrent);
    }
}

void CodeCompletionView::previousCompletion()
{
    QModelIndex firstCurrent = model()->index(model()->rowCount() - 1, 0);
    QModelIndex oldCurrent = currentIndex();
    QModelIndex current = moveCursor(MoveUp, Qt::NoModifier);

    if (current != oldCurrent && current.isValid()) {
        setCurrentIndex(current);
    } else {
        if (firstCurrent.isValid())
            setCurrentIndex(firstCurrent);
    }
}
