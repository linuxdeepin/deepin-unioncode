// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "namevaluemodel.h"

class NameValueModelPrivate
{
public:
    int indexOf(const QString &name);
    int findInsertPosition(const QString &name);

public:
    QVariantMap itemMap;
};

int NameValueModelPrivate::indexOf(const QString &name)
{
    for (int i = 0; i < itemMap.size(); ++i) {
        if (itemMap.keys()[i] == name)
            return i;
    }

    return -1;
}

int NameValueModelPrivate::findInsertPosition(const QString &name)
{
    auto iter = itemMap.cbegin();
    int i = 0;
    for (; iter != itemMap.cend(); ++iter, ++i) {
        if (iter.key().compare(name, Qt::CaseSensitive) > 0)
            return i;
    }

    return itemMap.size();
}

NameValueModel::NameValueModel(QObject *parent)
    : QAbstractTableModel(parent),
      d(new NameValueModelPrivate)
{
}

NameValueModel::~NameValueModel()
{
    delete d;
}

int NameValueModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return d->itemMap.size();
}

int NameValueModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 2;
}

QVariant NameValueModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= d->itemMap.size())
        return QVariant();

    auto name = d->itemMap.keys()[index.row()];
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
        if (index.column() == 0)
            return name;

        if (index.column() == 1)
            return d->itemMap.value(name);
        break;
    }
    return QVariant();
}

bool NameValueModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    // ignore changes to already set values:
    if (data(index, role) == value)
        return true;

    const QString oldName = data(this->index(index.row(), 0, QModelIndex())).toString();
    const QString oldValue = data(this->index(index.row(), 1, QModelIndex()), Qt::EditRole).toString();
    if (index.column() == 0) {
        //fail if a variable with the same name already exists
        const QString &newName = value.toString();
        if (newName.isEmpty() || newName.contains('='))
            return false;
        // Does the new name exist already?
        if (d->itemMap.contains(newName) || newName.isEmpty())
            return false;

        removeItem(index);
        QModelIndex newIndex = addItem(newName, oldValue);   // add the new variable
        emit focusIndex(newIndex);   // hint to focus on the name
        return true;
    } else if (index.column() == 1) {
        d->itemMap[oldName] = value;
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags NameValueModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

QVariant NameValueModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || role != Qt::DisplayRole)
        return QVariant();
    return section == 0 ? tr("Variable") : tr("Value");
}

void NameValueModel::setItems(const QVariantMap &items)
{
    beginResetModel();
    d->itemMap = items;
    endResetModel();
}

QVariantMap NameValueModel::items() const
{
    return d->itemMap;
}

QModelIndex NameValueModel::addItem()
{
    return addItem(tr("<VARIABLE>"), tr("<VALUE>"));
}

QModelIndex NameValueModel::addItem(const QString &name, const QVariant &value)
{
    int pos = d->indexOf(name);
    if (pos >= 0 && pos < d->itemMap.size())
        return index(pos, 0, QModelIndex());

    int insertPos = d->findInsertPosition(name);
    beginInsertRows(QModelIndex(), insertPos, insertPos);
    d->itemMap.insert(name, value);
    endInsertRows();

    return index(insertPos, 0, QModelIndex());
}

void NameValueModel::removeItem(const QModelIndex &index)
{
    auto name = data(this->index(index.row(), 0, QModelIndex())).toString();
    removeItem(name);
}

void NameValueModel::removeItem(const QString &variable)
{
    int pos = d->indexOf(variable);
    if (pos < 0)
        return;

    beginRemoveRows(QModelIndex(), pos, pos);
    d->itemMap.remove(variable);
    endRemoveRows();
}

void NameValueModel::clear()
{
    beginResetModel();
    d->itemMap.clear();
    endResetModel();
}

QString NameValueModel::variableFromIndex(const QModelIndex &index) const
{
    auto it = std::next(d->itemMap.cbegin(), index.row());
    return it.key();
}

QModelIndex NameValueModel::indexFromVariable(const QString &name) const
{
    int pos = d->indexOf(name);
    if (pos == -1)
        return {};

    return index(pos, 0);
}
