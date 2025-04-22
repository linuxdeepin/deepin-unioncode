// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakeitemmodel.h"

#include <QToolTip>
#include <QDebug>
#include <QCheckBox>

bool isTrue(const QString &value)
{
    const QString lower = value.toLower();
    return lower == QStringLiteral("true") || lower == QStringLiteral("on")
            || lower == QStringLiteral("1") || lower == QStringLiteral("yes");
}

void switchState(CMakeItem &item, bool state)
{
    auto value = item.value.toString();
    if (state) {
        if (value == "FALSE")
            item.value = "TRUE";
        else if (value == "OFF")
            item.value = "ON";
        else if (value == "NO")
            item.value = "YES";
        else
            item.value = "1";
    } else {
        if (value == "TRUE")
            item.value = "FALSE";
        else if (value == "ON")
            item.value = "OFF";
        else if (value == "YES")
            item.value = "NO";
        else
            item.value = "0";
    }
}

CMakeItem::CMakeItem()
{
}

CMakeItem::CMakeItem(const QByteArray *key, const QByteArray *value)
{
    this->key = QString::fromStdString(key->toStdString());
    this->value = QVariant(*value);
}

CMakeItem::itemType CMakeItem::dataToType(const QByteArray &data)
{
    if (data == "STRING")
        return itemType::STRING;
    if (data == "BOOL")
        return itemType::BOOL;
    if (data == "FILEPATH")
        return itemType::FILEPATH;
    if (data == "PATH")
        return itemType::PATH;
    if (data == "STATIC")
        return itemType::STATIC;
    if (data == "INTERNAL" || data == "UNINITIALIZED")
        return itemType::INTERNAL;

    return itemType::INTERNAL;
}

QString CMakeItem::typeToString(itemType type)
{
    switch (type) {
    case itemType::STRING:
        return QString("STRING");
        break;
    case itemType::BOOL:
        return QString("BOOL");
        break;
    case itemType::STATIC:
        return QString("STAITC");
        break;
    case itemType::PATH:
        return QString("PATH");
        break;
    case itemType::FILEPATH:
        return QString("FILEPATH");
        break;
    case itemType::INTERNAL:
        return QString("INTERNAL");
        break;
    default:
        break;
    }
}

CMakeItemModel::CMakeItemModel(QObject *parent) : QAbstractTableModel(parent)
{
}

CMakeItemModel::~CMakeItemModel()
{

}

int CMakeItemModel::rowCount(const QModelIndex &) const
{
    return cmakeData.size();
}

int CMakeItemModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant CMakeItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= cmakeData.size())
        return QVariant();

    auto item = cmakeData.at(index.row());
    switch (role) {
    case Qt::CheckStateRole:
        if (item.type == CMakeItem::BOOL && index.column() == columnType::Value) {
            if (isTrue(item.value.toString()))
                return Qt::Checked;
            return Qt::Unchecked;
        }
        break;
    case Qt::EditRole:
        return item.type == CMakeItem::BOOL ? isTrue(item.value.toString()) : item.value;
    case Qt::ToolTipRole:
        if (index.column() == columnType::Value)
            QToolTip::showText(QCursor::pos(), item.value.toString() + '\n' + item.description);
        else
            QToolTip::showText(QCursor::pos(), item.key + '\n' + item.description);
        break;
    case Qt::DisplayRole:
        if (index.column() == columnType::Key)
            return item.key;
        else if (index.column() == columnType::Value) {
            return item.value.toString();
        }
        break;
    default:
        break;
    }

    return QVariant();
}

QVariant CMakeItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        case Qt::DisplayRole:
        {
            if (orientation == Qt::Horizontal) {
                switch (section) {
                case Key:
                    return QObject::tr("Key");
                case Value:
                    return QObject::tr("Value");
                    break;
                }
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags CMakeItemModel::flags(const QModelIndex &index) const
{
    if (index.row() > cmakeData.size() || index.column() == columnType::Key)
        return QAbstractItemModel::flags(index);

    QFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    auto item = cmakeData.at(index.row());
    if (item.type == CMakeItem::BOOL)
        flags |= Qt::ItemIsUserCheckable;
    else
        flags |= Qt::ItemIsEditable;
    return flags;
}

bool CMakeItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() > cmakeData.size())
        return false;
    auto item = cmakeData.at(index.row());
    if (role == Qt::CheckStateRole && index.column() == columnType::Value)
        switchState(item, value.toInt() != Qt::Unchecked);
    if (role == Qt::EditRole)
        item.value = value;

    item.isChanged = true;
    cmakeData.replace(index.row(), item);
    emit dataChanged(index, index);
    return true;
}

void CMakeItemModel::setData(const QList<CMakeItem> &data)
{
    beginResetModel();
    cmakeData.clear();
    for (auto item : data) {
        if (showAdvanced && item.isAdvanced)
            cmakeData.append(item);
        else if (item.type != CMakeItem::STATIC
                 && item.type != CMakeItem::INTERNAL
                 && !item.isAdvanced)
            cmakeData.append(item);
    }
    endResetModel();
}

CMakeItem CMakeItemModel::itemOfIndex(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() > cmakeData.size())
        return CMakeItem();

    return cmakeData.at(index.row());
}

QList<CMakeItem> CMakeItemModel::getItems()
{
    return cmakeData;
}
