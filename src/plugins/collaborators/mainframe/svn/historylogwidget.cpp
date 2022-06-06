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
#include "historylogwidget.h"
#include "historyview.h"
#include "filemodifyview.h"

#include <QTextBrowser>

HistoryLogWidget::HistoryLogWidget(QWidget *parent)
    : QSplitter (parent)
    , descBrowse(new QTextBrowser)
    , hisView(new HistoryView)
    , changedView(new FileModifyView)
{
    descBrowse->setMinimumHeight(60);
    descBrowse->setPlaceholderText(QTextBrowser::tr("Description from revision log"));
    hisView->setMinimumHeight(300);
    changedView->setMinimumHeight(300);
    setOrientation(Qt::Vertical);
    addWidget(descBrowse);
    setCollapsible(0, false);
    addWidget(changedView);
    addWidget(hisView);
    setCollapsible(1, false);
    setHandleWidth(2);
    QObject::connect(hisView, &HistoryView::clicked, [=](const QModelIndex &index){
        descBrowse->setText(hisView->description(index.row()));
        changedView->setFiles(hisView->revisionFiles(index.row()));
    });
}

QTextBrowser *HistoryLogWidget::descriptionBrowse()
{
    return descBrowse;
}

HistoryView *HistoryLogWidget::historyView()
{
    return hisView;
}

FileModifyView *HistoryLogWidget::fileChangedView()
{
    return changedView;
}
