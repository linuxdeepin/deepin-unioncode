// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchresulttreeview.h"

#include "common/util/eventdefinitions.h"

#include <DFileIconProvider>

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

QStringList SearchResultModel::fileList() const
{
    return resultData.keys();
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
    int pos = resultData.keys().indexOf(group);
    auto parent = index(pos, 0);

    beginInsertRows(parent, pos, pos + itemList.size());
    resultData[group].append(itemList);
    endInsertRows();
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
    default:
        break;
    }

    return QVariant();
}

QVariant SearchResultModel::data(const QString &group, int role) const
{
    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
        return group;
    case Qt::DecorationRole:
        return DFileIconProvider::globalProvider()->icon(QFileInfo(group));
    default:
        break;
    }

    return QVariant();
}

class SearchResultTreeViewPrivate
{
    SearchResultTreeViewPrivate() {}
    ~SearchResultTreeViewPrivate();

    SearchResultModel model;
    friend class SearchResultTreeView;
};

SearchResultTreeViewPrivate::~SearchResultTreeViewPrivate()
{
}

SearchResultTreeView::SearchResultTreeView(QWidget *parent)
    : DTreeView(parent), d(new SearchResultTreeViewPrivate())
{
    setModel(&d->model);
    auto font = this->font();
    font.setFamily("Noto Mono");
    setFont(font);

    connect(this, &DTreeView::doubleClicked, this, [=](const QModelIndex &index) {
        auto item = d->model.findItem(index);
        if (!item)
            return;

        editor.gotoPosition(item->filePathName, item->line, item->column);
    });
}

SearchResultTreeView::~SearchResultTreeView()
{
    delete d;
}

void SearchResultTreeView::appendData(const FindItemList &itemList)
{
    d->model.appendResult(itemList);
}

void SearchResultTreeView::clearData()
{
    d->model.clear();
}

QStringList SearchResultTreeView::resultFileList() const
{
    return d->model.fileList();
}
