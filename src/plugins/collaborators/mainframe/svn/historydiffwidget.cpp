/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
