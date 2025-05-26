// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTITEMMODEL_H
#define PROJECTITEMMODEL_H

#include "itemnode.h"
#include "gui/projecttreeview.h"

#include <QStandardItemModel>

class ProjectItemModelPrivate;
class ProjectItemModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit ProjectItemModel(ProjectTreeView *parent = nullptr);
    ~ProjectItemModel();

    void setRootProjectNode(ProjectNode *rootNode);
    void setRootItem(NodeItem *root);
    NodeItem *rootItem() const;
    void updateProjectNode(ProjectNode *prjNode);
    void clear();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    int columnCount(const QModelIndex &index) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Node *nodeForIndex(const QModelIndex &index) const;
    NodeItem *itemForNode(const Node *node) const;
    NodeItem *itemForIndex(const QModelIndex &index) const;
    QModelIndex indexForNode(const Node *node) const;

private:
    ProjectItemModelPrivate *const d;
};

#endif   // PROJECTITEMMODEL_H
