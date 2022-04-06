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
#include "ldparser.h"

#include "ldparser.h"
#include "task.h"

#include "qtcassert.h"

namespace {
    // opt. drive letter + filename: (2 brackets)
    const char * const FILE_PATTERN = "(([A-Za-z]:)?[^:]+\\.[^:]+):";
    // line no. or elf segment + offset (1 bracket)
    const char * const POSITION_PATTERN = "(\\S+|\\(\\..+?[+-]0x[a-fA-F0-9]+\\)):";
    const char * const COMMAND_PATTERN = "^(.*[\\\\/])?([a-z0-9]+-[a-z0-9]+-[a-z0-9]+-)?(ld|gold)(-[0-9\\.]+)?(\\.exe)?: ";
    const char *const RANLIB_PATTERN = "ranlib(.exe)?: (file: (.*) has no symbols)$";
}

const char TASK_CATEGORY_COMPILE[] = "Task.Category.Compile";

LdParser::LdParser()
{
    setObjectName(QLatin1String("LdParser"));
    ranlib.setPattern(QLatin1String(RANLIB_PATTERN));
    QTC_CHECK(ranlib.isValid());
    regExpLinker.setPattern(QLatin1Char('^') +
                              QString::fromLatin1(FILE_PATTERN) + QLatin1Char('(') +
                              QString::fromLatin1(FILE_PATTERN) + QLatin1String(")?(") +
                              QLatin1String(POSITION_PATTERN) + QLatin1String(")?\\s(.+)$"));
    QTC_CHECK(regExpLinker.isValid());

    regExpGccNames.setPattern(QLatin1String(COMMAND_PATTERN));
    QTC_CHECK(regExpGccNames.isValid());
}

void LdParser::stdError(const QString &line)
{
    QString lne = rightTrimmed(line);
    if (lne.startsWith(QLatin1String("TeamBuilder "))
            || lne.startsWith(QLatin1String("distcc["))
            || lne.contains(QLatin1String("ar: creating "))) {
        IOutputParser::stdError(line);
        return;
    }

    if (lne.startsWith(QLatin1String("collect2:"))) {
        Task task = Task(Task::Error,
                         lne /* description */,
                         Utils::FileName() /* filename */,
                         -1 /* linenumber */,
                         TASK_CATEGORY_COMPILE);
        emit addTask(task, 1);
        return;
    }

    QRegularExpressionMatch match = ranlib.match(lne);
    if (match.hasMatch()) {
        QString description = match.captured(2);
        Task task(Task::Warning, description,
                  Utils::FileName(), -1,
                  TASK_CATEGORY_COMPILE);
        emit addTask(task, 1);
        return;
    }

    match = regExpGccNames.match(lne);
    if (match.hasMatch()) {
        QString description = lne.mid(match.capturedLength());
        Task::TaskType type = Task::Error;
        if (description.startsWith(QLatin1String("warning: "))) {
            type = Task::Warning;
            description = description.mid(9);
        } else if (description.startsWith(QLatin1String("fatal: ")))  {
            description = description.mid(7);
        }
        Task task(type, description, Utils::FileName() /* filename */, -1 /* line */,
                  TASK_CATEGORY_COMPILE);
        emit addTask(task, 1);
        return;
    }

    match = regExpLinker.match(lne);
    if (match.hasMatch()) {
        bool ok;
        int lineno = match.captured(7).toInt(&ok);
        if (!ok)
            lineno = -1;
        Utils::FileName filename = Utils::FileName::fromUserInput(match.captured(1));
        const QString sourceFileName = match.captured(4);
        if (!sourceFileName.isEmpty()
            && !sourceFileName.startsWith(QLatin1String("(.text"))
            && !sourceFileName.startsWith(QLatin1String("(.data"))) {
            filename = Utils::FileName::fromUserInput(sourceFileName);
        }
        QString description = match.captured(8).trimmed();
        Task::TaskType type = Task::Error;
        if (description.startsWith(QLatin1String("At global scope")) ||
            description.startsWith(QLatin1String("At top level")) ||
            description.startsWith(QLatin1String("instantiated from ")) ||
            description.startsWith(QLatin1String("In ")) ||
            description.startsWith(QLatin1String("first defined here")) ||
            description.startsWith(QLatin1String("note:"), Qt::CaseInsensitive)) {
            type = Task::Unknown;
        } else if (description.startsWith(QLatin1String("warning: "), Qt::CaseInsensitive)) {
            type = Task::Warning;
            description = description.mid(9);
        }
        Task task(type, description, filename, lineno, TASK_CATEGORY_COMPILE);
        emit addTask(task, 1);
        return;
    }

    IOutputParser::stdError(line);
}
