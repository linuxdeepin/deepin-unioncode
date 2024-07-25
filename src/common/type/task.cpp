// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "task.h"

unsigned int Task::s_nextId = 1;

static QIcon taskTypeIcon(Task::TaskType t)
{
    static QIcon icons[3] = {QIcon(),
                             QIcon::fromTheme("error"),
                             QIcon::fromTheme("warning")};

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
