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
#include "taskmodel.h"

#include <QFontMetrics>

TaskModel::TaskModel(QObject *parent) : QAbstractItemModel(parent)
{

}

QModelIndex TaskModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
        return QModelIndex();
    return createIndex(row, column);
}

QModelIndex TaskModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : tasks.count();
}

int TaskModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (!index.isValid() || row < 0 || row >= tasks.count() || index.column() != 0)
        return QVariant();

    if (role == TaskModel::File)
        return tasks.at(index.row()).file.toString();
    else if (role == TaskModel::Line)
        return tasks.at(index.row()).line;
    else if (role == TaskModel::MovedLine)
        return tasks.at(index.row()).movedLine;
    else if (role == TaskModel::Description)
        return tasks.at(index.row()).description;
    else if (role == TaskModel::Type)
        return static_cast<int>(tasks.at(index.row()).type);
    else if (role == TaskModel::Icon)
        return tasks.at(index.row()).icon;
    else if (role == TaskModel::Task_t)
        return QVariant::fromValue(task(index));
    return QVariant();
}

Task TaskModel::task(const QModelIndex &index) const
{
    int row = index.row();
    if (!index.isValid() || row < 0 || row >= tasks.count())
        return Task();
    return tasks.at(row);
}

QList<Task> TaskModel::getTasks() const
{
    return tasks;
}

bool sortById(const Task &task, unsigned int id)
{
    return task.taskId < id;
}

void TaskModel::addTask(const Task &task)
{
    auto it = std::lower_bound(tasks.begin(), tasks.end(),task.taskId, sortById);
    int i = it - tasks.begin();
    beginInsertRows(QModelIndex(), i, i);
    tasks.insert(it, task);
    endInsertRows();
}

void TaskModel::removeTask(const Task &task)
{
    int index = tasks.indexOf(task);
    if (index >= 0) {
        beginRemoveRows(QModelIndex(), index, index);
        tasks.removeAt(index);
        endRemoveRows();
    }
}

void TaskModel::clearTasks()
{
    if (tasks.isEmpty())
        return;

    beginRemoveRows(QModelIndex(), 0, tasks.count() -1);
    tasks.clear();
    endRemoveRows();
}

int TaskModel::taskCount()
{
    return tasks.count();
}

int TaskModel::sizeOfFile(const QFont &font)
{
    int count = tasks.count();
    if (count == 0)
        return 0;

    if (maxSizeOfFileName > 0 && font == fileMeasurementFont && lastMaxSizeIndex == count - 1)
        return maxSizeOfFileName;

    QFontMetrics fm(font);
    fileMeasurementFont = font;

    for (int i = lastMaxSizeIndex; i < count; ++i) {
        QString filename = tasks.at(i).file.toString();
        const int pos = filename.lastIndexOf(QLatin1Char('/'));
        if (pos != -1)
            filename = filename.mid(pos +1);

        maxSizeOfFileName = qMax(maxSizeOfFileName, fm.horizontalAdvance(filename));
    }
    lastMaxSizeIndex = count - 1;
    return maxSizeOfFileName;
}

int TaskModel::getSizeOfLineNumber(const QFont &font)
{
    if (sizeOfLineNumber == 0 || font != lineMeasurementFont) {
        QFontMetrics fm(font);
        lineMeasurementFont = font;
        const char *maxLineNumber = "99999";
        sizeOfLineNumber = fm.horizontalAdvance(QLatin1String(maxLineNumber));
    }
    return sizeOfLineNumber;
}
