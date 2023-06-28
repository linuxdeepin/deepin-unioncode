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

StackFrameView::StackFrameView(QWidget *parent)
    : QTreeView(parent)
{
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
