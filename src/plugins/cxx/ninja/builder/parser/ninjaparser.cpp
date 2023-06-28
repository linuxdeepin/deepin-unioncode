// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ninjaparser.h"

#include "services/builder/task.h"
#include "services/builder/fileutils.h"

const char TASK_CATEGORY_BUILDSYSTEM[] = "Task.Category.Buildsystem";

NinjaParser::NinjaParser()
{
    setObjectName(QLatin1String("NinjaParser"));
}

void NinjaParser::stdOutput(const QString &line, OutputPane::OutputFormat format)
{
    QString newContent = line;
    emit outputAdded(newContent, format);

    IOutputParser::stdOutput(newContent, format);
}

void NinjaParser::stdError(const QString &line)
{
    QString newContent = line;
    QRegExp exp("/.*:(\\d*):");
    int ret = newContent.indexOf(exp);
    QString filePath;
    int lineNumber = -1;
    if (ret != -1) {
        QStringList list = newContent.split(":");
        if (list.count() > 1) {
            filePath = list.at(0);
            lineNumber = list.at(1).toInt();
        }
    }

    Utils::FileName fileName;
    if (QFileInfo(filePath).isFile()) {
        fileName = Utils::FileName::fromUserInput(filePath);
    } else {
        fileName = Utils::FileName();
    }

    taskAdded(Task(Task::Error,
                   line,
                   fileName,
                   lineNumber,
                   TASK_CATEGORY_BUILDSYSTEM), 1, 0);
}

void NinjaParser::taskAdded(const Task &task, int linkedLines, int skippedLines)
{
    emit addTask(task, linkedLines, skippedLines);
}
