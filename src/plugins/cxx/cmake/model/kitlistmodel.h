// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KITLISTMODEL_H
#define KITLISTMODEL_H

#include "global_define.h"
#include "cmake/option/kit.h"

#include <QAbstractItemModel>

class KitListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit KitListModel(QObject *parent = nullptr);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Kit *itemForIndex(const QModelIndex &index);
    void setItemList(const QList<Kit> &itemList);
    QList<Kit> itemList() const { return kitItemList; }
    QModelIndex addItem();
    void removeItem(const QModelIndex &index);

private:
    QList<Kit> kitItemList;
};

#endif   // KITLISTMODEL_H
