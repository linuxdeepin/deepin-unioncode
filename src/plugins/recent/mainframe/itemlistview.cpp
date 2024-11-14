// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemlistview.h"
#include "common/common.h"
#include "itemdelegate.h"

#include <DStyledItemDelegate>
#include <DStandardItem>

#include <QToolTip>

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>

#define INIT_DATA "{\n    \"Projects\":[],\n    \"Documents\":[]\n}\n"

ItemListView::ItemListView(QWidget *parent)
    : DListView(parent), model(new QStandardItemModel(this))
{
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QListView::NoEditTriggers);
    setTextElideMode(Qt::ElideMiddle);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlternatingRowColors(false);
    setSelectionRectVisible(true);
    setMouseTracking(true);

    setUniformItemSizes(true);
    setResizeMode(Fixed);
    setOrientation(QListView::TopToBottom, false);
    setFrameStyle(QFrame::NoFrame);
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);

    setModel(model);
    setItemDelegate(new ItemDelegate(this));
}

ItemListView::~ItemListView()
{
}

void ItemListView::clear()
{
    model->clear();
}

void ItemListView::focusOutEvent(QFocusEvent *e)
{
    DListView::clearSelection();
    DListView::focusOutEvent(e);
}

void ItemListView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    return DListView::mousePressEvent(e);
}
