// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
