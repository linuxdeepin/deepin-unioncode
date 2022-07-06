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
#ifndef TASK_H
#define TASK_H

#include "fileutils.h"

#include <QObject>
#include <QIcon>

class Task
{
public:
    enum TaskType : char {
        Unknown,
        Error,
        Warning
    };

    enum Option : char {
        NoOptions   = 0,
        AddTextMark = 1 << 0,
        FlashWorthy = 1 << 1,
    };
    using Options = char;

    Task() = default;
    Task(TaskType type, const QString &description,
         const Utils::FileName &file, int line, QString category,
         const QIcon &icon = QIcon(),
         Options options = AddTextMark | FlashWorthy);

    bool isNull() const;
    void clear();

    unsigned int taskId = 0;
    TaskType type = Unknown;
    Options options = AddTextMark | FlashWorthy;
    QString description;
    Utils::FileName file;
    int line = -1;
    int movedLine = -1; // contains a line number if the line was moved in the editor
    QString category;
    QIcon icon;

signals:

public slots:

private:
    static unsigned int s_nextId;
};

bool operator==(const Task &t1, const Task &t2);

Q_DECLARE_METATYPE(Task)

#endif // TASK_H
