// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localtreemodel.h"

#include "localtreeitem.h"
#include "dap/protocol.h"

#include <QStringList>

using namespace DEBUG_NAMESPACE;
using namespace dap;
LocalTreeModel::LocalTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new LocalTreeItem(this);
}

LocalTreeModel::~LocalTreeModel()
{
    delete rootItem;
}

void LocalTreeModel::setHeaders(const QList<QString> &headers)
{
    this->headers = headers;
}

int LocalTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<LocalTreeItem*>(parent.internalPointer())->columnCount();
    return headers.count();
}

QVariant LocalTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    LocalTreeItem *item = static_cast<LocalTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags LocalTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant LocalTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return headers.at(section);

    return QVariant();
}

QModelIndex LocalTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    LocalTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<LocalTreeItem*>(parent.internalPointer());

    LocalTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex LocalTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    LocalTreeItem *childItem = static_cast<LocalTreeItem*>(index.internalPointer());
    LocalTreeItem *parentItem = childItem->getParentItem();

    if (!parentItem || parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int LocalTreeModel::rowCount(const QModelIndex &parent) const
{
    LocalTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<LocalTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void LocalTreeModel::appendItem(LocalTreeItem* parent, IVariables &vars)
{
    if (parent) {
        for (auto var : vars) {
            LocalTreeItem *item = new LocalTreeItem(this, parent);
            item->setVariable(var.var);
            parent->appendChild(item);

            if (var.children.size() > 0) {
                appendItem(item, var.children);
            }
        }
    }
};

void LocalTreeModel::setDatas(IVariables &datas)
{
    clear();

    appendItem(rootItem, datas);
}

void LocalTreeModel::clear()
{
    if (rootItem) {
        beginRemoveRows(rootItem->index(), 0, rootItem->childCount() - 1);
        rootItem->removeChildren();
        endRemoveRows();
    }
}

QModelIndex LocalTreeModel::indexForItem(const LocalTreeItem *item) const
{
    if (!item || item == rootItem) {
        return QModelIndex();
    }

    LocalTreeItem *p = item->parent();
    if (!p)
        return QModelIndex();

    auto mitem = const_cast<LocalTreeItem *>(item);
    int row = p->indexOf(mitem);
    return createIndex(row, 0, mitem);
}
