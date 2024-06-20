// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localtreeitem.h"
#include "localtreemodel.h"

#include <QStringList>
#include <qabstractitemmodel.h>

LocalTreeItem::LocalTreeItem(LocalTreeModel *_model, LocalTreeItem *parent)
    : model(_model)
{
    parentItem = parent;
}

LocalTreeItem::~LocalTreeItem()
{
    qDeleteAll(children);
}

void LocalTreeItem::appendChild(LocalTreeItem *item)
{
    children.append(item);
}

void LocalTreeItem::setVariable(dap::Variable &variable)
{
    itemVariable = variable;
}

LocalTreeItem *LocalTreeItem::child(int row)
{
    return children.value(row);
}

int LocalTreeItem::childCount() const
{
    return children.count();
}

int LocalTreeItem::columnCount() const
{
    return KReference + 1;
}

QVariant LocalTreeItem::data(int column) const
{
    QVariant var;
    switch (column) {
    case ColumnType::kName:
    {
        QString name = itemVariable.name.c_str();
        var = name;
        break;
    }
    case ColumnType::KVaule:
    {
        QString value = itemVariable.value.c_str();
        var = value;
        break;
    }
    case ColumnType::KReference:
        if (itemVariable.memoryReference) {
            QString value = itemVariable.memoryReference.value().c_str();
            var = value;
            break;
        }
    }
    return var;
}

LocalTreeItem *LocalTreeItem::getParentItem()
{
    return parentItem;
}

void LocalTreeItem::removeChildren()
{
    if (childCount() == 0)
        return;
    if (model) {
        clear();
    }
}

LocalTreeItem *LocalTreeItem::parent() const
{
    return parentItem;
}

int LocalTreeItem::indexOf(const LocalTreeItem *item) const
{
    auto it = std::find(begin(), end(), item);
    return it == end() ? -1 : static_cast<int>(it - begin());
}

QModelIndex LocalTreeItem::index() const
{
    if (!model)
        return QModelIndex();
    return model->indexForItem(this);
}

void LocalTreeItem::clear()
{
    while (childCount() != 0) {
        LocalTreeItem *item = children.takeLast();
        item->model = nullptr;
        item->parentItem = nullptr;
        delete item;
        item = nullptr;
    }
}

int LocalTreeItem::row() const
{
    if (parentItem)
        return parentItem->children.indexOf(const_cast<LocalTreeItem*>(this));

    return 0;
}
