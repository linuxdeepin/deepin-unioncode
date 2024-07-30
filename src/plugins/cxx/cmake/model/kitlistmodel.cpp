// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kitlistmodel.h"

#include <QUuid>

KitListModel::KitListModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

int KitListModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant KitListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= kitItemList.size())
        return {};

    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
        return kitItemList.value(index.row()).kitName();
    default:
        break;
    }

    return {};
}

QModelIndex KitListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (column == 0 && row < kitItemList.size()) {
        const auto &item = kitItemList.at(row);
        return createIndex(row, 0, const_cast<Kit *>(&item));
    }

    return {};
}

QModelIndex KitListModel::parent(const QModelIndex &child) const
{
    return {};
}

int KitListModel::rowCount(const QModelIndex &parent) const
{
    return kitItemList.size();
}

Kit *KitListModel::itemForIndex(const QModelIndex &index)
{
    return static_cast<Kit *>(index.internalPointer());
}

void KitListModel::setItemList(const QList<Kit> &itemList)
{
    beginResetModel();
    kitItemList = itemList;
    endResetModel();
}

QModelIndex KitListModel::addItem()
{
    Kit item;
    item.setKitName(tr("Unnamed"));
    item.setId(QUuid::createUuid().toString());

    int pos = kitItemList.size();
    beginInsertRows(QModelIndex(), pos, pos);
    kitItemList.append(item);
    endInsertRows();

    return index(pos, 0);
}

void KitListModel::removeItem(const QModelIndex &index)
{
    auto item = itemForIndex(index);
    if (!item)
        return;

    int pos = kitItemList.indexOf(*item);
    beginRemoveRows(QModelIndex(), pos, pos);
    kitItemList.removeAt(pos);
    endRemoveRows();
}
