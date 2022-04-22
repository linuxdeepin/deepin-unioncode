#include "commithistorymodel.h"

namespace collaborators {

CommitHistoryModel::CommitHistoryModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    mColumns.insert(CommitDataRole::TreeViewIcon, QAbstractItemModel::tr(""));
    mColumns.insert(CommitDataRole::Graph, QAbstractItemModel::tr(""));
    mColumns.insert(CommitDataRole::Sha, QAbstractItemModel::tr("Sha"));
    mColumns.insert(CommitDataRole::Log, QAbstractItemModel::tr("History"));
    mColumns.insert(CommitDataRole::Author, QAbstractItemModel::tr("Author"));
    mColumns.insert(CommitDataRole::Date, QAbstractItemModel::tr("Date"));
}

QVariant CommitHistoryModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QVariant CommitHistoryModel::headerData(int s, Qt::Orientation o, int role) const
{
    return QVariant();
}

QModelIndex CommitHistoryModel::index(int r, int c, const QModelIndex &par) const
{
    return {};
}

QModelIndex CommitHistoryModel::parent(const QModelIndex &index) const
{
    return index;
}

int CommitHistoryModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

bool CommitHistoryModel::hasChildren(const QModelIndex &par) const
{
    return false;
}

int CommitHistoryModel::columnCount(const QModelIndex &) const
{
    return mColumns.count();
}

} //namespace
