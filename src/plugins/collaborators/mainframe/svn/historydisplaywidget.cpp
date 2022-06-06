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
#include "historydisplaywidget.h"
#include "historylogwidget.h"
#include "historydiffwidget.h"

#include <QTextBrowser>

HistoryDisplayWidget::HistoryDisplayWidget(QWidget *parent)
    : QSplitter (parent)
    , hisLogWidget (new HistoryLogWidget)
    , hisDiffWidget (new HistoryDiffWidget)
{
    hisLogWidget->setMinimumWidth(300);
    hisDiffWidget->setMinimumWidth(300);
    setOrientation(Qt::Horizontal);
    setHandleWidth(2);
    addWidget(hisLogWidget);
    setCollapsible(0, false);
    addWidget(hisDiffWidget);
    setCollapsible(1, false);
}

HistoryLogWidget *HistoryDisplayWidget::logWidget()
{
    return hisLogWidget;
}

HistoryDiffWidget *HistoryDisplayWidget::diffWidget()
{
    return hisDiffWidget;
}
