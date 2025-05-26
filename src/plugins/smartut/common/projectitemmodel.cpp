// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectitemmodel.h"
#include "utils/utils.h"

#include <QFile>

class ProjectItemModelPrivate
{
public:
    QStandardItem *findChildItem(QStandardItem *item, const Node *node);
    void addFolderNode(NodeItem *parent, FolderNode *folderNode, QSet<Node *> *seen);
    void syncProjectItems(NodeItem *targetItem, NodeItem *srcItem);

public:
    NodeItem *rootItem { nullptr };
    ProjectTreeView *view { nullptr };
};

QStandardItem *ProjectItemModelPrivate::findChildItem(QStandardItem *item, const Node *node)
{
    auto find = [node](QStandardItem *item) {
        NodeItem *witem = static_cast<NodeItem *>(item);
        return witem ? witem->itemNode == node : false;
    };

    if (item) {
        if (find(item))
            return item;

        for (int i = 0; i < item->rowCount(); ++i) {
            if (auto found = findChildItem(item->child(i), node))
                return found;
        }
    }

    return nullptr;
}

void ProjectItemModelPrivate::addFolderNode(NodeItem *parent, FolderNode *folderNode, QSet<Node *> *seen)
{
    for (Node *node : folderNode->nodes()) {
        if (FolderNode *subFolderNode = node->asFolderNode()) {
            int oldSize = seen->size();
            seen->insert(subFolderNode);
            if (seen->size() > oldSize) {
                auto node = new NodeItem(subFolderNode);
                parent->appendRow(node);
                addFolderNode(node, subFolderNode, seen);
                // TODO: sort
            } else {
                addFolderNode(parent, subFolderNode, seen);
            }
        } else if (FileNode *fileNode = node->asFileNode()) {
            int oldSize = seen->size();
            seen->insert(fileNode);
            if (seen->size() > oldSize) {
                auto node = new NodeItem(fileNode);
                parent->appendRow(node);
            }
        }
    }
}

void ProjectItemModelPrivate::syncProjectItems(NodeItem *targetItem, NodeItem *srcItem)
{
    if (!targetItem || !srcItem)
        return;

    QHash<QString, NodeItem *> targetChildren;
    for (int i = 0; i < targetItem->rowCount(); ++i) {
        NodeItem *child = static_cast<NodeItem *>(targetItem->child(i));
        if (child)
            targetChildren.insert(child->filePath(), child);
    }

    for (int i = 0; i < srcItem->rowCount(); ++i) {
        NodeItem *sourceChild = static_cast<NodeItem *>(srcItem->child(i));
        if (!sourceChild)
            continue;

        QString key = sourceChild->filePath();
        if (targetChildren.contains(key)) {
            syncProjectItems(targetChildren[key], sourceChild);
            targetChildren.remove(key);
        } else {
            auto newItem = new NodeItem(sourceChild->itemNode);
            targetItem->appendRow(newItem);
            if (sourceChild->hasChildren()) {
                QSet<Node *> seen;
                addFolderNode(newItem, newItem->itemNode->asFolderNode(), &seen);
            }
        }
    }

    for (NodeItem *item : qAsConst(targetChildren)) {
        if (item->state != Generating)
            targetItem->removeRow(item->row());
    }
}

ProjectItemModel::ProjectItemModel(ProjectTreeView *parent)
    : QStandardItemModel(parent),
      d(new ProjectItemModelPrivate())
{
    d->view = parent;
}

ProjectItemModel::~ProjectItemModel()
{
    clear();
    delete d;
}

void ProjectItemModel::setRootProjectNode(ProjectNode *rootNode)
{
    setRootItem(new NodeItem(rootNode));

    QSet<Node *> seen;
    d->addFolderNode(d->rootItem, rootNode, &seen);
}

void ProjectItemModel::setRootItem(NodeItem *root)
{
    d->rootItem = root;
    appendRow(d->rootItem);
}

NodeItem *ProjectItemModel::rootItem() const
{
    return d->rootItem;
}

void ProjectItemModel::updateProjectNode(ProjectNode *prjNode)
{
    QSet<Node *> seen;
    NodeItem item(prjNode);
    d->addFolderNode(&item, prjNode, &seen);
    d->syncProjectItems(d->rootItem, &item);
}

void ProjectItemModel::clear()
{
    while (hasChildren()) {
        removeRow(0);
    }
    d->rootItem = nullptr;
}

QVariant ProjectItemModel::data(const QModelIndex &index, int role) const
{
    const Node *const node = nodeForIndex(index);
    if (!node)
        return {};

    switch (role) {
    case Qt::DisplayRole:
        return node->displayName();
    case Qt::ToolTipRole: {
        QString tips = node->tooltip();
        if (node->isFileNodeType() && !QFile::exists(node->filePath()))
            tips += tr(" [Ungenerated]");
        return tips;
    }
    case Qt::DecorationRole:
        return node->icon();
    case Qt::FontRole: {
        QFont font;
        if (node->isProjectNodeType())
            font.setBold(true);
        return font;
    }
    case Qt::ForegroundRole:
        if (!QFile::exists(node->filePath())) {
            auto fore = d->view->palette().color(QPalette::Text);
            fore.setAlpha(qRound(255 * 0.4));
            return fore;
        }
        break;
    case ItemStateRole: {
        auto item = itemForIndex(index);
        if (item)
            return item->state;
    } break;
    default:
        break;
    }

    return QStandardItemModel::data(index, role);
}

int ProjectItemModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid())
        return d->rootItem->rowCount();
    const auto *item = itemFromIndex(index);
    return item ? item->rowCount() : 0;
}

int ProjectItemModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return 1;
}

Qt::ItemFlags ProjectItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (Node *node = nodeForIndex(index)) {
        if (node->isProjectNodeType())
            return flags;
        if (d->view->viewType() == ProjectTreeView::Project) {
            flags |= Qt::ItemIsUserCheckable;
            if (!node->isFileNodeType())
                flags |= Qt::ItemIsUserTristate;
        }
    }

    return flags;
}

Node *ProjectItemModel::nodeForIndex(const QModelIndex &index) const
{
    NodeItem *item = itemForIndex(index);
    return item ? item->itemNode : nullptr;
}

NodeItem *ProjectItemModel::itemForNode(const Node *node) const
{
    auto item = d->findChildItem(d->rootItem, node);
    return static_cast<NodeItem *>(item);
}

NodeItem *ProjectItemModel::itemForIndex(const QModelIndex &index) const
{
    return static_cast<NodeItem *>(itemFromIndex(index));
}

QModelIndex ProjectItemModel::indexForNode(const Node *node) const
{
    NodeItem *item = itemForNode(node);
    return item ? indexFromItem(item) : QModelIndex();
}
