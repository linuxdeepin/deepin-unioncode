// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKFILTERMODEL_H
#define TASKFILTERMODEL_H

#include "taskmodel.h"

namespace ReverseDebugger{
namespace Internal {

class TaskFilterModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    TaskFilterModel(TaskModel *sourceModel, QObject *parent = nullptr);

    TaskModel *taskModel() { return sourceModel; }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QList<QString> filteredCategories() const { return categoryIds; }
    void setFilteredCategories(const QList<QString> &_categoryIds) {
        categoryIds = _categoryIds;
        invalidateFilter();
    }
    void setSortType(int type) {
        sortType = type;
        invalidateFilter();
    }
    void setEventTid(int _tid) {
        //NOTE: tid filter can combine with event type filter
        tid = _tid;
        invalidateFilter();
    }
    void setEventRange(int begin, int end) {
        eventBegin = begin;
        eventEnd = end;
        eventIndexBegin = -1;
        eventIndexEnd = -1;
        invalidateFilter();
    }
    void setEventIndexRange(int begin, int end) {
        eventBegin = -1;
        eventEnd = -1;
        eventIndexBegin = begin;
        eventIndexEnd = end;
        invalidateFilter();
    }

    Task task(const QModelIndex &index) const
    { return sourceModel->task(mapToSource(index)); }

    QModelIndex mapFromSource(const QModelIndex &idx) const;
signals:

public slots:

private:
    void handleNewRows(const QModelIndex &index, int first, int last);
    void handleRowsAboutToBeRemoved(const QModelIndex &index, int first, int last);
    void handleDataChanged(const QModelIndex &top, const QModelIndex &bottom);
    void handleReset();

    QModelIndex mapToSource(const QModelIndex &index) const;
    void invalidateFilter();
    void updateMapping() const;
    bool filterAcceptsTask(const Task &task) const;

    int sortType = 0;
    int tid = 0;
    int eventBegin = 0;
    int eventEnd = 0;
    int eventIndexBegin = 0;
    int eventIndexEnd = 0;
    QList<QString> categoryIds;

    mutable QList<int> rowMapping;

    TaskModel *sourceModel = nullptr;
};

} // namespace Internal
} // namespace ReverseDebugger

#endif // TASKFILTERMODEL_H
