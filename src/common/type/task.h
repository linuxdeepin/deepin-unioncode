// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASK_H
#define TASK_H

#include "common/util/fileutils.h"

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
