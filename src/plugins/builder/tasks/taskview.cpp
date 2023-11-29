// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskview.h"
#include "taskdelegate.h"

#include <DScrollBar>

TaskView::TaskView(QWidget *parent) : DListView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QFontMetrics fm(font());
    int vStepSize = fm.height() + 3;
    if (vStepSize < TaskDelegate::Positions::minimumHeight())
        vStepSize = TaskDelegate::Positions::minimumHeight();

    verticalScrollBar()->setSingleStep(vStepSize);
}

TaskView::~TaskView() = default;

void TaskView::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    static_cast<TaskDelegate *>(itemDelegate())->emitSizeHintChanged(selectionModel()->currentIndex());
}
