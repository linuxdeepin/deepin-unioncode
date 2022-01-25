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
#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "debug.h"
#include "variable.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class LocalTreeItem;
class LocalTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit LocalTreeModel(QObject *parent = nullptr);
    ~LocalTreeModel() override;

    void setHeaders(const QList<QString> &headers);
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Custumize functions.
    void setDatas(IVariables &datas);
    void clear();

    QModelIndex indexForItem(const LocalTreeItem *needle) const;

private:
    void appendItem(LocalTreeItem* parent, IVariables &vars);

    LocalTreeItem *rootItem = nullptr;
    QList<LocalTreeItem *> items;
    QStringList headers;
};

#endif // TREEMODEL_H
