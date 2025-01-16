// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localtreemodel.h"

#include "localtreeitem.h"
#include "dap/protocol.h"

#include <QStringList>
#include <QToolTip>
#include <QWriteLocker>
#include <QReadLocker>

using namespace DEBUG_NAMESPACE;
using namespace dap;
QReadWriteLock mutex;

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
        return static_cast<LocalTreeItem *>(parent.internalPointer())->columnCount();
    return headers.count();
}

QVariant LocalTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    LocalTreeItem *item = static_cast<LocalTreeItem *>(index.internalPointer());
    QReadLocker locker(&mutex);
    if (!items.contains(item))
        return QVariant();
    locker.unlock();

    if (role == Qt::ToolTipRole)
        QToolTip::showText(QCursor::pos(), item->data(index.column()).toString());

    if(role == Qt::ForegroundRole && item->hasUpdated() == true && index.column() == 1)
        return QVariant(QColor(Qt::red));

    if (role != Qt::DisplayRole)
        return QVariant();

    if (!item)
        return QVariant();

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
        parentItem = static_cast<LocalTreeItem *>(parent.internalPointer());

    LocalTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex LocalTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    LocalTreeItem *childItem = static_cast<LocalTreeItem *>(index.internalPointer());

    if (!childItem || !items.contains(childItem))
        return QModelIndex();

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

    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<LocalTreeItem *>(parent.internalPointer());
        if (!items.contains(parentItem))
            return 0;
    }

    QReadLocker locker(&mutex);
    if (parentItem && (items.contains(parentItem) || parentItem == rootItem ))
        return parentItem->childCount();
    return 0;
}

bool LocalTreeModel::hasChildren(const QModelIndex &parent) const
{
    LocalTreeItem *parentItem;

    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<LocalTreeItem *>(parent.internalPointer());
        if (!items.contains(parentItem))
            return false;
    }

    return parentItem->hasChildren();
}

void LocalTreeModel::clearHighlightItems()
{
    if (rootItem)
        rootItem->setChildrenUpdated(false);
}

void LocalTreeModel::appendItem(LocalTreeItem *parent, const IVariables &vars)
{
    if (!items.contains(parent) && parent != rootItem)
        return;

    if (parent) {
        QList<LocalTreeItem *> newItems;

        // when fetchChild do not highlight variables.
        // append item to a parent which has child, highLight new item
        bool fetchChild = parent->childCount() == 0;

        for (auto var : vars) {
            auto item = parent->updateVariable(var.var);
            //item = nullptr : parent has no child equal this var
            if (!item) {
                beginInsertColumns(parent->index(), parent->childCount(), 1);
                item = new LocalTreeItem(this, parent);
                item->setUpdated(!fetchChild);
                item->setVariable(var.var);
                parent->appendChild(item);

                items.append(item);

                endInsertRows();
            }

            newItems.append(item);
        }
        parent->removeRedundantItems(newItems);
    }
}

void LocalTreeModel::setDatas(IVariables &datas)
{
    QWriteLocker locker(&mutex);
    appendItem(rootItem, datas);
}

void LocalTreeModel::clear()
{
    QWriteLocker locker(&mutex);
    items.clear();
    locker.unlock();

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
