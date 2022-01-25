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
#ifndef TREEITEM_H
#define TREEITEM_H

#include "dap/protocol.h"

#include <QList>
#include <QVariant>
#include <QVector>

enum ColumnType {
    kName,
    KVaule,
    KReference
};

class LocalTreeModel;
class LocalTreeItem
{
public:
    explicit LocalTreeItem(LocalTreeModel *model, LocalTreeItem *parentItem = nullptr);
    ~LocalTreeItem();

    void appendChild(LocalTreeItem *child);
    void setVariable(dap::Variable &variable);

    LocalTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    LocalTreeItem *getParentItem();
    void removeChildren();
    LocalTreeItem *parent() const;
    int indexOf(const LocalTreeItem *item) const;

    QModelIndex index() const;

private:
    using const_iterator = QVector<LocalTreeItem *>::const_iterator;
    const_iterator begin() const { return children.begin(); }
    const_iterator end() const { return children.end(); }

    void clear();

    QVector<LocalTreeItem*> children;
    dap::Variable itemVariable;
    LocalTreeItem *parentItem = nullptr;
    LocalTreeModel *model = nullptr;
};

#endif // TREEITEM_H
