// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localtreeitem.h"
#include "localtreemodel.h"

#include <QStringList>
#include <qabstractitemmodel.h>

LocalTreeItem::LocalTreeItem(LocalTreeModel *_model, LocalTreeItem *parent)
    : model(_model)
{
    parentItem = parent;
    QObject::connect(this, &LocalTreeItem::updateChildVariables, model, &LocalTreeModel::updateChildVariables);
}

LocalTreeItem::~LocalTreeItem()
{
    qDeleteAll(children);
}

void LocalTreeItem::appendChild(LocalTreeItem *item)
{
    children.append(item);
}

bool LocalTreeItem::hasUpdated()
{
    return updated;
}

void LocalTreeItem::setUpdated(bool updated)
{
    this->updated = updated;
}

void LocalTreeItem::setChildrenUpdated(bool updated)
{
    if (childCount() > 0) {
        auto it = children.begin();
        while (it != children.end()) {
            (*it)->setUpdated(updated);
            if ((*it)->childCount() > 0)
                (*it)->setChildrenUpdated(updated);
            it++;
        }
    }
}

LocalTreeItem *LocalTreeItem::updateVariable(dap::Variable &var)
{
    auto it = std::find_if(children.begin(), children.end(), [&](const auto &child) {
        return child->itemVariable.name == var.name;
    });

    if (it != children.end()) {
        auto child = *it;
        bool updated = child->itemVariable.value != var.value;
        child->setVariable(var);
        child->updated = updated;

        if (child->childrenFetched) {
            child->setChildrenFetched(false);
            emit updateChildVariables(child->index());
        }

        emit model->dataChanged(index(), index());
        return child;
    }

    return nullptr;
}

void LocalTreeItem::removeRedundantItems(const QList<LocalTreeItem *> &itemList)
{
    auto it = children.begin();
    while (it != children.end()) {
        if (!itemList.contains(*it))
            children.removeOne(*it);
        else
            it++;
    }
    emit model->layoutChanged();
}

void LocalTreeItem::setVariable(const dap::Variable &variable)
{
    itemVariable = variable;
}

LocalTreeItem *LocalTreeItem::child(int row)
{
    return children.value(row);
}

int LocalTreeItem::childCount() const
{
    return children.count();
}

int LocalTreeItem::columnCount() const
{
    return kType + 1;
}

QVariant LocalTreeItem::data(int column) const
{
    QVariant var;
    switch (column) {
    case ColumnType::kName:
    {
        QString name = itemVariable.name.c_str();
        var = name;
        break;
    }
    case ColumnType::KVaule:
    {
        QString value = itemVariable.value.c_str();
        var = value;
        break;
    }
    case ColumnType::kType:
    {
        QString type = itemVariable.type.value().c_str();
        var = type;
        break;
    }
    }
//    case ColumnType::KReference:
//        if (itemVariable.memoryReference) {
//            QString value = itemVariable.memoryReference.value().c_str();
//            var = value;
//            break;
//        }
//    }
    return var;
}

LocalTreeItem *LocalTreeItem::getParentItem()
{
    return parentItem;
}

void LocalTreeItem::removeChildren()
{
    if (childCount() == 0)
        return;
    if (model) {
        clear();
    }
}

LocalTreeItem *LocalTreeItem::parent() const
{
    return parentItem;
}

int LocalTreeItem::indexOf(const LocalTreeItem *item) const
{
    auto it = std::find(begin(), end(), item);
    return it == end() ? -1 : static_cast<int>(it - begin());
}

QModelIndex LocalTreeItem::index() const
{
    if (!model)
        return QModelIndex();
    return model->indexForItem(this);
}

void LocalTreeItem::clear()
{
    while (childCount() != 0) {
        LocalTreeItem *item = children.takeLast();
        item->model = nullptr;
        item->parentItem = nullptr;
        delete item;
        item = nullptr;
    }
}

int LocalTreeItem::row() const
{
    if (parentItem)
        return parentItem->children.indexOf(const_cast<LocalTreeItem*>(this));

    return 0;
}

bool LocalTreeItem::hasChildren()
{
    return (itemVariable.variablesReference != 0) || (childCount() > 0);
}

int64_t LocalTreeItem::childReference()
{
    return itemVariable.variablesReference;
}

void LocalTreeItem::setChildrenFetched(bool fetch)
{
    childrenFetched = fetch;
}

bool LocalTreeItem::canFetchChildren()
{
    return !childrenFetched;
}
