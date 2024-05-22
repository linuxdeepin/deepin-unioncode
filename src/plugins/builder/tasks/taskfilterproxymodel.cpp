// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskfilterproxymodel.h"

TaskFilterProxyModel::TaskFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool TaskFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    if (!index.isValid())
        return false;

    int taskType = static_cast<int>(sourceModel()->data(index, TaskModel::Type).toInt());

    switch (filterType) {
    case ShowType::All:
        return true;
    case ShowType::Error:
        return taskType == Error;
    case ShowType::Warning:
        return taskType == Warning;
    default:
        return false;
    }
}

void TaskFilterProxyModel::setFilterType(ShowType showType)
{
    this->filterType = showType;
    invalidateFilter();
}
