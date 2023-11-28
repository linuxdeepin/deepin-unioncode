// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stackframeview.h"
#include "interface/stackframemodel.h"

#include <QDebug>
#include <QFontMetrics>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

StackFrameView::StackFrameView(QWidget *parent)
    : DTreeView(parent)
{
    initHeaderView();
    setHeader(headerView);
    setTextElideMode(Qt::TextElideMode::ElideLeft);
    setFrameStyle(QFrame::NoFrame);

    connect(this, &QAbstractItemView::activated,
            this, &StackFrameView::rowActivated);
    connect(this, &QAbstractItemView::clicked,
            this, &StackFrameView::rowClicked);
}

StackFrameView::~StackFrameView()
{
}

void StackFrameView::rowActivated(const QModelIndex &index)
{
    model()->setData(index, QVariant(), ItemActivatedRole);
}

void StackFrameView::rowClicked(const QModelIndex &index)
{
    model()->setData(index, QVariant(), ItemClickedRole);
}

void StackFrameView::initHeaderView()
{
    headerView = new QHeaderView(Qt::Orientation::Horizontal);
    headerView->setDefaultSectionSize(68);
    headerView->setDefaultAlignment(Qt::AlignLeft);
    headerView->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
}
