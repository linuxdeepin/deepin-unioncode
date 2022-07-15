/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "mavenparser.h"

#include "services/builder/task.h"
#include "services/builder/fileutils.h"

const char TASK_CATEGORY_BUILDSYSTEM[] = "Task.Category.Buildsystem";

MavenParser::MavenParser()
{
    setObjectName(QLatin1String("MavenParser"));
}

void MavenParser::stdOutput(const QString &line, OutputFormat format)
{
    QString newContent = line;
    QRegExp exp("\\033\\[(\\d*;*\\d*)m");
    newContent.replace(exp, "");

    if (newContent.indexOf("[ERROR]") != -1) {
        format = OutputFormat::ErrorMessage;
        stdError(newContent);
    }

    emit outputAdded(newContent, format);

    IOutputParser::stdOutput(newContent, format);
}

void MavenParser::stdError(const QString &line)
{
    QString newContent = line;
    QRegExp exp("/.*:\\[(\\d*),(\\d*)\\]");
    int pos = newContent.indexOf(exp);
    QString filePath;
    int lineNumber = -1;
    if (pos != -1) {
        QStringList list = newContent.mid(pos).split(":");
        if (list.count() > 1) {
            filePath = list.at(0);
            QString last = list.at(1);
            QStringList sublist = last.split(",");
            if (sublist.count() > 1) {
                lineNumber = sublist[0].mid(1).toInt();
            }
        }
    } else {
        QRegExp pomExp("Non-parseable POM /.*:");
        QString header = "Non-parseable POM ";
        pos = newContent.indexOf(pomExp);
        if (pos != -1) {
            newContent = newContent.mid(pos);
            pos = newContent.indexOf(":");
            if (pos != -1) {
                filePath = newContent.left(pos).mid(header.length() - 1);
                newContent = newContent.mid(pos);
                pos = newContent.indexOf("@ line ");
                if (pos != -1) {
                    newContent = newContent.mid(pos);
                    pos = newContent.indexOf(",");
                    if (pos != -1) {
                        newContent = newContent.left(pos);
                        header = "@ line ";
                        lineNumber = newContent.mid(header.length() - 1).toInt();
                    }
                }
            }
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

void MavenParser::taskAdded(const Task &task, int linkedLines, int skippedLines)
{
    emit addTask(task, linkedLines, skippedLines);
}
