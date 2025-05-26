// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskmodel.h"

#include <QIcon>
#include <QFontMetrics>

extern void *kTimeline;

namespace ReverseDebugger {
namespace Internal {

namespace Core {
static QString Id()
{
    return "";
}
}

uint Task::s_nextId = 1;

ReverseDebugger::Internal::Task::Task(
        const QString &_description, const QString &_category, const EventEntry *_event)
    : taskId(s_nextId), event(_event), category(_category), description(_description)
{
    ++s_nextId;
}

bool Task::isNull() const
{
    return taskId == 0;
}

void Task::clear()
{
    taskId = 0;
    description.clear();
    category.clear();
}

bool operator==(const Task &t1, const Task &t2)
{
    return t1.taskId == t2.taskId;
}

bool operator<(const Task &a, const Task &b)
{
    return a.taskId < b.taskId;
}

uint qHash(const Task &task)
{
    return task.taskId;
}

TaskModel::TaskModel(QObject *parent)
{
    Q_UNUSED(parent)
}

void TaskModel::setTimelinePtr(void *timeline)
{
    this->timeline = timeline;
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
    return parent.isValid() ? 0 : allTasks.count();
}

int TaskModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (!index.isValid() || row < 0 || row >= allTasks.count() || index.column() != 0)
        return QVariant();

    if (role == TaskModel::Description)
        return allTasks.at(row).description;
    else if (role == TaskModel::Return)
        return (long long)(allTasks.at(row).event->syscall_result);
    else if (role == TaskModel::ThreadNum)
        return allTasks.at(row).event->thread_num;
    else if (role == TaskModel::Tid)
        return allTasks.at(row).event->tid;
    else if (role == TaskModel::Time)
        return allTasks.at(row).event->time;
    else if (role == TaskModel::Duration)
        return allTasks.at(row).event->duration;
    else if (role == TaskModel::Category)
        //        return m_tasks.at(row).category.uniqueIdentifier();
        return allTasks.at(row).category;
    else if (role == TaskModel::Icon)
        return QIcon();
    else if (role == TaskModel::ExtraInfo) {
        QString info;
        bool ok;
        int i = 0;
        auto task = allTasks.at(row);
        for (; i < task.description.size(); ++i) {
            if (task.description[i] > QLatin1Char('9')) {
                break;
            }
        }
        if (i > 0) {
            i = task.description.left(i).toInt(&ok, 10);
            if (ok) {
                char buf[EVENT_EXTRA_INFO_SIZE];
                if (get_event_extra_info(kTimeline,
                                         i, buf, sizeof(buf))
                    > 0) {
                    info = QLatin1String(buf);
                }

                if (task.event->syscall_result < 0) {
                    info += QLatin1Char('(');
                    info += QLatin1String(errno_name(static_cast<int>(task.event->syscall_result)));
                    info += QLatin1Char(')');
                }
            }
        }
        return info;
    }
    return QVariant();
}

Task TaskModel::task(int row) const
{
    if (row < 0 || row >= allTasks.count())
        return Task();
    return allTasks.at(row);
}

Task TaskModel::task(const QModelIndex &index) const
{
    int row = index.row();
    if (!index.isValid() || row < 0 || row >= allTasks.count())
        return Task();
    return allTasks.at(row);
}

QList<QString> TaskModel::categoryIds() const
{
    QList<QString> categoryList = categories.keys();
    categoryList.removeAll("");   // remove global category we added for bookkeeping
    return categoryList;
}

QString TaskModel::categoryDisplayName(const QString &categoryId) const
{
    return categories.value(categoryId).displayName;
}

void TaskModel::addCategory(const QString &categoryId, const QString &categoryName)
{
    if (categoryId.isEmpty())
        return;

    CategoryData data;
    data.displayName = categoryName;
    categories.insert(categoryId, data);
}

QList<Task> TaskModel::tasks(const QString &categoryId) const
{
    if (categoryId.isEmpty())
        return allTasks;

    QList<Task> taskList;
    foreach (const Task &t, allTasks) {
        if (t.category == categoryId)
            taskList.append(t);
    }
    return taskList;
}

bool sortById(const Task &task, unsigned int id)
{
    return task.taskId < id;
}

void TaskModel::addTask(const Task &task)
{
    Q_ASSERT(categories.keys().contains(task.category));
    CategoryData &data = categories[task.category];
    CategoryData &global = categories[""];

    QList<Task>::iterator it = std::lower_bound(allTasks.begin(), allTasks.end(), task.taskId, sortById);
    int i = it - allTasks.begin();
    beginInsertRows(QModelIndex(), i, i);
    allTasks.insert(it, task);
    data.addTask(task);
    global.addTask(task);
    endInsertRows();
}

void TaskModel::removeTask(const Task &task)
{
    int index = allTasks.indexOf(task);
    if (index >= 0) {
        const Task &t = allTasks.at(index);

        beginRemoveRows(QModelIndex(), index, index);
        categories[task.category].removeTask(t);
        categories[Core::Id()].removeTask(t);
        allTasks.removeAt(index);
        endRemoveRows();
    }
}

void TaskModel::clearTasks(const QString &categoryId)
{
    typedef QHash<QString, CategoryData>::ConstIterator IdCategoryConstIt;

    if (categoryId.isEmpty()) {
        if (allTasks.count() == 0)
            return;
        beginRemoveRows(QModelIndex(), 0, allTasks.count() - 1);
        allTasks.clear();
        const IdCategoryConstIt cend = categories.constEnd();
        for (IdCategoryConstIt it = categories.constBegin(); it != cend; ++it)
            categories[it.key()].clear();
        endRemoveRows();
    } else {
        int index = 0;
        int start = 0;
        CategoryData &global = categories[Core::Id()];
        CategoryData &cat = categories[categoryId];

        while (index < allTasks.count()) {
            while (index < allTasks.count() && allTasks.at(index).category != categoryId) {
                ++start;
                ++index;
            }
            if (index == allTasks.count())
                break;
            while (index < allTasks.count() && allTasks.at(index).category == categoryId)
                ++index;

            // Index is now on the first non category
            beginRemoveRows(QModelIndex(), start, index - 1);

            for (int i = start; i < index; ++i) {
                global.removeTask(allTasks.at(i));
                cat.removeTask(allTasks.at(i));
            }

            allTasks.erase(allTasks.begin() + start, allTasks.begin() + index);

            endRemoveRows();
            index = start;
        }
    }
}

int TaskModel::getSizeOfLineNumber(const QFont &font)
{
    if (sizeOfLineNumber == 0 || font != lineMeasurementFont) {
        QFontMetrics fm(font);
        lineMeasurementFont = font;
        sizeOfLineNumber = fm.horizontalAdvance(QLatin1String("88888"));
    }
    return sizeOfLineNumber;
}

int TaskModel::taskCount(const QString &categoryId)
{
    return categories.value(categoryId).count;
}

int TaskModel::rowForId(unsigned int id)
{
    QList<Task>::const_iterator it = std::lower_bound(allTasks.constBegin(), allTasks.constEnd(), id, sortById);
    if (it == allTasks.constEnd())
        return -1;
    return it - allTasks.constBegin();
}

}   // namespace Internal
}   // namespace ReverseDebugger
