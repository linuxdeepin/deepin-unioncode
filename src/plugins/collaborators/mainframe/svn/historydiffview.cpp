// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
