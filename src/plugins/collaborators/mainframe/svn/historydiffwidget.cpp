// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "historydiffwidget.h"
#include "historydiffview.h"

#include "DiffInfo.h"
#include "DiffHelper.h"

HistoryDiffWidget::HistoryDiffWidget(QWidget *parent)
    : QSplitter (parent)
    , oldView (new HistoryDiffView(HistoryDiffView::tr("Old File")))
    , newView (new HistoryDiffView(HistoryDiffView::tr("New File")))
{
    setStyleSheet("QSplitter{background-color: #2E2F30;}");
    oldView->setMinimumWidth(100);
    newView->setMinimumWidth(100);

    addWidget(oldView);
    setCollapsible(0, false);
    addWidget(newView);
    setCollapsible(1, false);
    setHandleWidth(2);
}

HistoryDiffView *HistoryDiffWidget::getOldView() const
{
    return oldView;
}

HistoryDiffView *HistoryDiffWidget::getNewView() const
{
    return newView;
}
