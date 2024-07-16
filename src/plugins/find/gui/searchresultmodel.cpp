// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchresultmodel.h"

#include <DFileIconProvider>

#include <QRegularExpression>

DWIDGET_USE_NAMESPACE

SearchResultModel::SearchResultModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

int SearchResultModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const
{
    if (auto item = findItem(index))
        return data(*item, role);

    QString group = findGroup(index);
    if (!group.isEmpty())
        return data(group, role);

    return QVariant();
}

QModelIndex SearchResultModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column == 0 && parent.isValid()) {
        QString group = findGroup(parent);
        if (!group.isEmpty()) {
            auto items = resultData.value(group);
            if (row < items.count()) {
                auto &item = items.at(row);
                return createIndex(row, 0, const_cast<FindItem *>(&item));
            }
        }
    } else if (column == 0 && row < resultData.size()) {
        return createIndex(row, 0);
    }

    return QModelIndex();
}

QModelIndex SearchResultModel::parent(const QModelIndex &child) const
{
    if (auto tool = findItem(child)) {
        int groupIndex = 0;
        for (const auto &itemsInGroup : resultData) {
            if (itemsInGroup.contains(*tool))
                return index(groupIndex, 0);
            ++groupIndex;
        }
    }
    return QModelIndex();
}

int SearchResultModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return resultData.size();

    if (findItem(parent))
        return 0;

    QString group = findGroup(parent);
    if (!group.isEmpty())
        return resultData.value(group).count();

    return 0;
}

Qt::ItemFlags SearchResultModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void SearchResultModel::clear()
{
    beginResetModel();
    resultData.clear();
    endResetModel();
}

void SearchResultModel::setReplaceText(const QString &text, bool regex)
{
    replaceText = text;
    enableRegex = regex;
}

FindItem *SearchResultModel::findItem(const QModelIndex &index) const
{
    auto item = static_cast<FindItem *>(index.internalPointer());
    return item;
}

QString SearchResultModel::findGroup(const QModelIndex &index) const
{
    if (index.isValid() && !index.parent().isValid() && index.column() == 0 && index.row() >= 0) {
        const QList<QString> &keys = resultData.keys();
        if (index.row() < keys.count())
            return keys.at(index.row());
    }

    return QString();
}

void SearchResultModel::appendResult(const FindItemList &list)
{
    QMap<QString, FindItemList> result;
    for (const auto &item : list) {
        if (!resultData.contains(item.filePathName)) {
            if (result.contains(item.filePathName)) {
                addItem(item.filePathName, result[item.filePathName]);
                result.clear();
            }
            addGroup(item.filePathName);
            result[item.filePathName].append(item);
        } else {
            result[item.filePathName].append(item);
        }
    }

    for (auto iter = result.begin(); iter != result.end(); ++iter)
        addItem(iter.key(), iter.value());
}

QMap<QString, FindItemList> SearchResultModel::allResult() const
{
    return resultData;
}

QMap<QString, FindItemList> SearchResultModel::findResult(const QModelIndex &index) const
{
    QMap<QString, FindItemList> resultMap;
    if (hasChildren(index)) {
        const auto &group = findGroup(index);
        resultMap.insert(group, resultData[group]);
    } else {
        auto item = findItem(index);
        if (!item)
            return {};

        resultMap.insert(item->filePathName, { *item });
    }

    return resultMap;
}

void SearchResultModel::remove(const QModelIndex &index)
{
    if (hasChildren(index)) {
        const auto &group = findGroup(index);
        removeGroup(group);
    } else {
        auto item = findItem(index);
        if (!item)
            return;
        removeItem(item->filePathName, *item);
    }
}

void SearchResultModel::addGroup(const QString &group)
{
    QList<QString> groupList = resultData.keys();
    groupList.append(group);
    std::stable_sort(std::begin(groupList), std::end(groupList));
    int pos = groupList.indexOf(group);

    beginInsertRows(QModelIndex(), pos, pos);
    resultData.insert(group, FindItemList());
    endInsertRows();
}

void SearchResultModel::addItem(const QString &group, const FindItemList &itemList)
{
    int row = resultData.keys().indexOf(group);
    auto parent = index(row, 0);
    int pos = resultData[group].count();

    beginInsertRows(parent, pos, pos + itemList.size());
    resultData[group].append(itemList);
    endInsertRows();
}

void SearchResultModel::removeGroup(const QString &group)
{
    if (!resultData.contains(group))
        return;

    int pos = resultData.keys().indexOf(group);
    beginRemoveRows(QModelIndex(), pos, pos);
    resultData.remove(group);
    endRemoveRows();
}

void SearchResultModel::removeItem(const QString &group, const FindItem &item)
{
    if (!resultData.contains(group) || !resultData[group].contains(item))
        return;

    if (resultData[group].count() == 1)
        return removeGroup(group);

    int row = resultData.keys().indexOf(group);
    auto parent = index(row, 0);
    int pos = resultData[group].indexOf(item);

    beginRemoveRows(parent, pos, pos);
    resultData[group].removeOne(item);
    endRemoveRows();
}

QVariant SearchResultModel::data(const FindItem &item, int role) const
{
    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
        return item.context;
    case LineRole:
        return item.line;
    case ColumnRole:
        return item.column;
    case KeywordRole:
        return item.keyword;
    case MatchedTextRole:
        return item.matchedText;
    case ReplaceTextRole:
        if (!item.capturedTexts.isEmpty())
            return Utils::expandRegExpReplacement(replaceText, item.capturedTexts);
        return replaceText;
    default:
        break;
    }

    return QVariant();
}

QVariant SearchResultModel::data(const QString &group, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return QFileInfo(group).fileName();
    case FilePathRole:
    case Qt::ToolTipRole:
        return group;
    case Qt::DecorationRole:
        return DFileIconProvider::globalProvider()->icon(QFileInfo(group));
    default:
        break;
    }

    return QVariant();
}
