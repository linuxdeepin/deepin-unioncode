/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<huangyub@uniontech.com>
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
#include "task.h"

unsigned int Task::s_nextId = 1;

static QIcon taskTypeIcon(Task::TaskType t)
{
    static QIcon icons[3] = {QIcon(),
                             QIcon(":/resources/icons/error.png"),
                             QIcon(":/resources/icons/attention.svg")};

    if (t < 0 || t > 2)
        t = Task::Unknown;

    return icons[t];
}

Task::Task(TaskType type_, const QString &description_,
           const Utils::FileName &file_, int line_, QString category_,
           const QIcon &icon, Options options) :
    taskId(s_nextId), type(type_), options(options), description(description_),
    file(file_), line(line_), movedLine(line_), category(category_),
    icon(icon.isNull() ? taskTypeIcon(type_) : icon)
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
    type = Task::Unknown;
    description.clear();
    file = Utils::FileName();
    line = -1;
    movedLine = -1;
    category = "";
    icon = QIcon();
}

bool operator==(const Task &t1, const Task &t2)
{
    return t1.taskId == t2.taskId;
}
