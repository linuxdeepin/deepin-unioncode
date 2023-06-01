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
#include "taskfiltermodel.h"

namespace ReverseDebugger {
namespace Internal {

TaskFilterModel::TaskFilterModel(TaskModel *sourceModel, QObject *parent) : QAbstractItemModel(parent),
    sourceModel(sourceModel)
{
    Q_ASSERT(sourceModel);
    updateMapping();

    connect(sourceModel, &QAbstractItemModel::rowsInserted,
            this, &TaskFilterModel::handleNewRows);
    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &TaskFilterModel::handleRowsAboutToBeRemoved);
    connect(sourceModel, &QAbstractItemModel::modelReset,
            this, &TaskFilterModel::handleReset);
    connect(sourceModel, &QAbstractItemModel::dataChanged,
            this, &TaskFilterModel::handleDataChanged);

    sortType = 0;
    tid = -1;
    eventBegin = -1;
    eventEnd = -1;
    eventIndexBegin = -1;
    eventIndexEnd = -1;
}
QModelIndex TaskFilterModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
        return QModelIndex();
    return createIndex(row, column);
}

QModelIndex TaskFilterModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int TaskFilterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return rowMapping.count();
}

int TaskFilterModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return sourceModel->columnCount(parent);
}

QVariant TaskFilterModel::data(const QModelIndex &index, int role) const
{
    return sourceModel->data(mapToSource(index), role);
}

QModelIndex TaskFilterModel::mapFromSource(const QModelIndex &idx) const
{
    QList<int>::const_iterator it = std::lower_bound(rowMapping.constBegin(), rowMapping.constEnd(), idx.row());
    if (it == rowMapping.constEnd())
        return QModelIndex();
    return index(it - rowMapping.constBegin(), 0);
}

void TaskFilterModel::handleNewRows(const QModelIndex &index, int first, int last)
{
    if (index.isValid())
        return;

    QList<int> newMapping;
    for (int i = first; i <= last; ++i) {
        const Task &task = sourceModel->task(sourceModel->index(i, 0));
        if (filterAcceptsTask(task))
            newMapping.append(i);
    }

    const int newItems = newMapping.count();
    if (!newItems)
        return;

    int filteredFirst = -1;
    if (last == sourceModel->rowCount() - 1)
        filteredFirst = rowMapping.count();
    else
        filteredFirst = std::lower_bound(rowMapping.constBegin(), rowMapping.constEnd(), first) - rowMapping.constBegin();

    const int filteredLast = filteredFirst + newItems - 1;
    beginInsertRows(QModelIndex(), filteredFirst, filteredLast);
    if (filteredFirst == rowMapping.count()) {
        rowMapping.append(newMapping);
    } else {
        QList<int> rest = rowMapping.mid(filteredFirst);

        rowMapping.reserve(rowMapping.count() + newItems);
        rowMapping.erase(rowMapping.begin() + filteredFirst, rowMapping.end());
        rowMapping.append(newMapping);
        foreach (int pos, rest)
            rowMapping.append(pos + newItems);
    }
    endInsertRows();
}

static QPair<int, int> findFilteredRange(int first, int last, const QList<int> &list)
{
    QList<int>::const_iterator filteredFirst = std::lower_bound(list.constBegin(), list.constEnd(), first);
    QList<int>::const_iterator filteredLast = std::upper_bound(filteredFirst, list.constEnd(), last);
    return qMakePair(filteredFirst - list.constBegin(), filteredLast - list.constBegin() - 1);
}


void TaskFilterModel::handleRowsAboutToBeRemoved(const QModelIndex &index, int first, int last)
{
    if (index.isValid())
        return;

    const QPair<int, int> range = findFilteredRange(first, last, rowMapping);
    if (range.first > range.second)
        return;

    beginRemoveRows(QModelIndex(), range.first, range.second);
    rowMapping.erase(rowMapping.begin() + range.first, rowMapping.begin() + range.second + 1);
    for (int i = range.first; i < rowMapping.count(); ++i)
        rowMapping[i] = rowMapping.at(i) - (last - first) - 1;
    endRemoveRows();
}

void TaskFilterModel::handleDataChanged(const QModelIndex &top, const QModelIndex &bottom)
{
    const QPair<int, int> range = findFilteredRange(top.row(), bottom.row(), rowMapping);
    if (range.first > range.second)
        return;

    emit dataChanged(index(range.first, top.column()), index(range.second, bottom.column()));
}

void TaskFilterModel::handleReset()
{
    invalidateFilter();
}

QModelIndex TaskFilterModel::mapToSource(const QModelIndex &index) const
{
    int row = index.row();
    if (row >= rowMapping.count())
        return QModelIndex();
    return sourceModel->index(rowMapping.at(row), index.column(), index.parent());
}

void TaskFilterModel::invalidateFilter()
{
    beginResetModel();
    updateMapping();
    endResetModel();
}

static const EventEntry* g_event_table = nullptr;

bool sort_duration(const int v1, const int v2)
{
    return g_event_table[v1].duration < g_event_table[v2].duration;
}

bool sort_result(const int v1, const int v2)
{
    return g_event_table[v1].syscall_result < g_event_table[v2].syscall_result;
}

bool sort_threads_number(const int v1, const int v2)
{
    return g_event_table[v1].thread_num < g_event_table[v2].thread_num;
}

void TaskFilterModel::updateMapping() const
{
    rowMapping.clear();
    for (int i = 0; i < sourceModel->rowCount(); ++i) {
        QModelIndex index = sourceModel->index(i, 0);
        const Task &task = sourceModel->task(index);
        if (filterAcceptsTask(task)) {
            if (tid > 0 && tid != task.event->tid) {
                continue;
            }

            if (eventIndexBegin >= 0 &&
                eventIndexEnd >= eventIndexBegin &&
                (i < eventIndexBegin || i > eventIndexEnd)){
                // not in index range!
                continue;
            }

            if (eventBegin >= 0 &&
                eventEnd >= eventBegin &&
                (task.event->type < eventBegin || task.event->type > eventEnd)){
                // not in event range!
                continue;
            }
            rowMapping.append(i);
        }
    }

    const Task &task = sourceModel->task(0);
    g_event_table = task.event;
    switch (sortType) {
    case 1:
        std::sort(rowMapping.begin(), rowMapping.end(), sort_duration);
        break;
    case 2:
        std::sort(rowMapping.begin(), rowMapping.end(), sort_result);
        break;
    case 3:
        std::sort(rowMapping.begin(), rowMapping.end(), sort_threads_number);
        break;
    default:
        break;
    }
}

bool TaskFilterModel::filterAcceptsTask(const Task &task) const
{
    bool accept = true;

    if (categoryIds.contains(task.category))
        accept = false;

    return accept;
}



} // namespace Internal
} // namespace ReverseDebugger

