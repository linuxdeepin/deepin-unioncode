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
#include "historydiffview.h"

#include "FileDiffView.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

HistoryDiffView::HistoryDiffView(const QString &title, QWidget *parent)
    : QWidget(parent)
    , vLayout (new QVBoxLayout)
    , titleLabel (new QLabel)
    , searchEdit (new QLineEdit)
    , diffView (new FileDiffView)
{
    titleLabel->setText(title);
    searchEdit->setPlaceholderText(QLineEdit::tr("Search Text"));
    vLayout->addWidget(titleLabel);
    vLayout->addWidget(searchEdit);
    vLayout->addWidget(diffView);
    vLayout->setSpacing(1);
    setLayout(vLayout);
}

FileDiffView *HistoryDiffView::getDiffView() const
{
    return diffView;
}

void HistoryDiffView::setTitle(const QString &title)
{
    titleLabel->setText(title);
}

QString HistoryDiffView::getTitle() const
{
    return titleLabel->text();
}
