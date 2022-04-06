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
#include "gccparser.h"

#include "ldparser.h"
#include "task.h"
#include "buildmanager.h"

#include "qtcassert.h"

#include <QTextLayout>

// opt. drive letter + filename: (2 brackets)
static const char FILE_PATTERN[] = "(<command[ -]line>|([A-Za-z]:)?[^:]+):";
static const char COMMAND_PATTERN[] = "^(.*?[\\\\/])?([a-z0-9]+-[a-z0-9]+-[a-z0-9]+-)?(gcc|g\\+\\+)(-[0-9\\.]+)?(\\.exe)?: ";

const char TASK_CATEGORY_COMPILE[] = "Task.Category.Compile";

GccParser::GccParser()
{
    setObjectName(QLatin1String("GCCParser"));
    regExp.setPattern(QLatin1Char('^') + QLatin1String(FILE_PATTERN)
                        + QLatin1String("(\\d+):(\\d+:)?\\s+((fatal |#)?(warning|error|note):?\\s)?([^\\s].+)$"));
    QTC_CHECK(regExp.isValid());

    regExpIncluded.setPattern(QString::fromLatin1("\\bfrom\\s") + QLatin1String(FILE_PATTERN)
                                + QLatin1String("(\\d+)(:\\d+)?[,:]?$"));
    QTC_CHECK(regExpIncluded.isValid());

    // optional path with trailing slash
    // optional arm-linux-none-thingy
    // name of executable
    // optional trailing version number
    // optional .exe postfix
    regExpGccNames.setPattern(QLatin1String(COMMAND_PATTERN));
    QTC_CHECK(regExpGccNames.isValid());

    appendOutputParser(new LdParser);
}

void GccParser::stdError(const QString &line)
{
    QString lne = rightTrimmed(line);

    // Blacklist some lines to not handle them:
    if (lne.startsWith(QLatin1String("TeamBuilder ")) ||
        lne.startsWith(QLatin1String("distcc["))) {
        IOutputParser::stdError(line);
        return;
    }

    // Handle misc issues:
    if (lne.startsWith(QLatin1String("ERROR:")) ||
        lne == QLatin1String("* cpp failed")) {
        newTask(Task(Task::Error,
                     lne /* description */,
                     Utils::FileName() /* filename */,
                     -1 /* linenumber */,
                     TASK_CATEGORY_COMPILE));
        return;
    }

    QRegularExpressionMatch match = regExpGccNames.match(lne);
    if (match.hasMatch()) {
        QString description = lne.mid(match.capturedLength());
        Task::TaskType type = Task::Error;
        if (description.startsWith(QLatin1String("warning: "))) {
            type = Task::Warning;
            description = description.mid(9);
        } else if (description.startsWith(QLatin1String("fatal: ")))  {
            description = description.mid(7);
        }
        Task task(type, description, Utils::FileName(), /* filename */
                  -1, /* line */ TASK_CATEGORY_COMPILE);
        newTask(task);
        return;
    }

    match = regExp.match(lne);
    if (match.hasMatch()) {
        Utils::FileName filename = Utils::FileName::fromUserInput(match.captured(1));
        int lineno = match.captured(3).toInt();
        Task::TaskType type = Task::Unknown;
        QString description = match.captured(8);
        if (match.captured(7) == QLatin1String("warning"))
            type = Task::Warning;
        else if (match.captured(7) == QLatin1String("error") ||
                 description.startsWith(QLatin1String("undefined reference to")) ||
                 description.startsWith(QLatin1String("multiple definition of")))
            type = Task::Error;
        // Prepend "#warning" or "#error" if that triggered the match on (warning|error)
        // We want those to show how the warning was triggered
        if (match.captured(5).startsWith(QLatin1Char('#')))
            description = match.captured(5) + description;

        Task task(type, description, filename, lineno, TASK_CATEGORY_COMPILE);
        newTask(task);
        return;
    }

    match = regExpIncluded.match(lne);
    if (match.hasMatch()) {
        newTask(Task(Task::Unknown,
                     lne.trimmed() /* description */,
                     Utils::FileName::fromUserInput(match.captured(1)) /* filename */,
                     match.captured(3).toInt() /* linenumber */,
                     TASK_CATEGORY_COMPILE));
        return;
    } else if (lne.startsWith(QLatin1Char(' '))) {
        amendDescription(lne, true);
        return;
    }

    doFlush();
    IOutputParser::stdError(line);
}

void GccParser::stdOutput(const QString &line)
{
    doFlush();
    IOutputParser::stdOutput(line);
}

QString GccParser::id()
{
    return "OutputParser.Gcc";
}

void GccParser::newTask(const Task &task)
{
    doFlush();
    currentTask = task;
    lines = 1;
}

void GccParser::doFlush()
{
    if (currentTask.isNull())
        return;
    Task t = currentTask;
    currentTask.clear();
    emit addTask(t, lines, 1);
    lines = 0;
}

void GccParser::amendDescription(const QString &desc, bool monospaced)
{
    if (currentTask.isNull())
        return;
    int start = currentTask.description.count() + 1;
    currentTask.description.append(QLatin1Char('\n'));
    currentTask.description.append(desc);
    if (monospaced) {
        QTextLayout::FormatRange fr;
        fr.start = start;
        fr.length = desc.count() + 1;
        fr.format.setFontStyleHint(QFont::Monospace);
    }
    ++lines;
    return;
}
