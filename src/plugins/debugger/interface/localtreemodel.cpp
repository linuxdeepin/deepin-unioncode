/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

    if (parentItem == rootItem)
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
