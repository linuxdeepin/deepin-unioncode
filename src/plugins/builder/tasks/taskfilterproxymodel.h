// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKFILTERPROXYMODEL_H
#define TASKFILTERPROXYMODEL_H

#include "taskmodel.h"

#include <QSortFilterProxyModel>

class TaskFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TaskFilterProxyModel(QObject *parent = nullptr);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    void setFilterType(ShowType filterType);

private:
    ShowType filterType = ShowType::All;

};

#endif // TASKFILTERPROXYMODEL_H
