// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREEITEM_H
#define TREEITEM_H

#include "dap/protocol.h"

#include <QList>
#include <QVariant>
#include <QVector>
#include <QMutex>

enum ColumnType {
    kName,
    KVaule,
    kType,
    KReference
};

class LocalTreeModel;
class LocalTreeItem : public QObject
{
    Q_OBJECT
public:
    explicit LocalTreeItem(LocalTreeModel *model, LocalTreeItem *parentItem = nullptr);
    ~LocalTreeItem();

    void removeRedundantItems(const QList<LocalTreeItem *> &itemList);
    void setUpdated(bool updated);
    void setChildrenUpdated(bool updated);
    LocalTreeItem *updateVariable(dap::Variable &var);
    void appendChild(LocalTreeItem *child);
    void setVariable(const dap::Variable &variable);

    LocalTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    LocalTreeItem *getParentItem();
    void removeChildren();
    LocalTreeItem *parent() const;
    int indexOf(const LocalTreeItem *item) const;
    bool hasChildren();
    int64_t childReference();
    bool hasUpdated();

    bool canFetchChildren();
    void setChildrenFetched(bool fetch);

    QModelIndex index() const;

signals:
    void updateChildVariables(const QModelIndex &index);

private:
    using const_iterator = QVector<LocalTreeItem *>::const_iterator;
    const_iterator begin() const { return children.begin(); }
    const_iterator end() const { return children.end(); }

    void clear();

    QVector<LocalTreeItem*> children;
    dap::Variable itemVariable;
    LocalTreeItem *parentItem = nullptr;
    LocalTreeModel *model = nullptr;
    bool childrenFetched = false;
    bool updated = false;
};

#endif // TREEITEM_H
