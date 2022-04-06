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
#include "gnumakeparser.h"

#include "task.h"

#include "qtcassert.h"
#include "fileutils.h"

#include <QDir>
#include <QFile>

const char TASK_CATEGORY_BUILDSYSTEM[] = "Task.Category.Buildsystem";

namespace {
    // optional full path, make executable name, optional exe extension, optional number in square brackets, colon space
    const char * const MAKEEXEC_PATTERN("^(.*?[/\\\\])?(mingw(32|64)-|g)?make(.exe)?(\\[\\d+\\])?:\\s");
    const char * const MAKEFILE_PATTERN("^((.*?[/\\\\])?[Mm]akefile(\\.[a-zA-Z]+)?):(\\d+):\\s");
}

GnuMakeParser::GnuMakeParser()
{
    setObjectName(QLatin1String("GnuMakeParser"));
    makeDir.setPattern(QLatin1String(MAKEEXEC_PATTERN) +
                         QLatin1String("(\\w+) directory .(.+).$"));
    QTC_CHECK(makeDir.isValid());
    makeLine.setPattern(QLatin1String(MAKEEXEC_PATTERN) + QLatin1String("(.*)$"));
    QTC_CHECK(makeLine.isValid());
    errorInMakefile.setPattern(QLatin1String(MAKEFILE_PATTERN) + QLatin1String("(.*)$"));
    QTC_CHECK(errorInMakefile.isValid());
}

void GnuMakeParser::setWorkingDirectory(const QString &workingDirectory)
{
    addDirectory(workingDirectory);
    IOutputParser::setWorkingDirectory(workingDirectory);
}

bool GnuMakeParser::hasFatalErrors() const
{
    return (fatalErrorCount > 0) || IOutputParser::hasFatalErrors();
}

void GnuMakeParser::stdOutput(const QString &line)
{
    const QString lne = rightTrimmed(line);

    QRegularExpressionMatch match = makeDir.match(lne);
    if (match.hasMatch()) {
        if (match.captured(6) == QLatin1String("Leaving"))
            removeDirectory(match.captured(7));
        else
            addDirectory(match.captured(7));
        return;
    }

    IOutputParser::stdOutput(line);
}

class Result {
public:
    Result() = default;

    QString description;
    bool isFatal = false;
    Task::TaskType type = Task::Error;
};

static Result parseDescription(const QString &description)
{
    Result result;
    if (description.startsWith(QLatin1String("warning: "), Qt::CaseInsensitive)) {
        result.description = description.mid(9);
        result.type = Task::Warning;
        result.isFatal = false;
    } else if (description.startsWith(QLatin1String("*** "))) {
        result.description = description.mid(4);
        result.type = Task::Error;
        result.isFatal = true;
    } else {
        result.description = description;
        result.type = Task::Error;
        result.isFatal = false;
    }
    return result;
}

void GnuMakeParser::stdError(const QString &line)
{
    const QString lne = rightTrimmed(line);

    QRegularExpressionMatch match = errorInMakefile.match(lne);
    if (match.hasMatch()) {
        flush();
        Result res = parseDescription(match.captured(5));
        if (res.isFatal)
            ++fatalErrorCount;
        if (!suppressIssues) {
            taskAdded(Task(res.type, res.description,
                           Utils::FileName::fromUserInput(match.captured(1)) /* filename */,
                           match.captured(4).toInt(), /* line */
                           TASK_CATEGORY_BUILDSYSTEM), 1, 0);
        }
        return;
    }
    match = makeLine.match(lne);
    if (match.hasMatch()) {
        flush();
        Result res = parseDescription(match.captured(6));
        if (res.isFatal)
            ++fatalErrorCount;
        if (!suppressIssues) {
            Task task = Task(res.type, res.description,
                             Utils::FileName() /* filename */, -1, /* line */
                             TASK_CATEGORY_BUILDSYSTEM);
            taskAdded(task, 1, 0);
        }
        return;
    }

    IOutputParser::stdError(line);
}

void GnuMakeParser::addDirectory(const QString &dir)
{
    if (dir.isEmpty())
        return;
    directories.append(dir);
}

void GnuMakeParser::removeDirectory(const QString &dir)
{
    directories.removeOne(dir);
}

void GnuMakeParser::taskAdded(const Task &task, int linkedLines, int skippedLines)
{
    Task editable(task);

    if (task.type == Task::Error) {
        // assume that all make errors will be follow up errors:
        suppressIssues = true;
    }

    QString filePath(task.file.toString());

    if (!filePath.isEmpty() && !QDir::isAbsolutePath(filePath)) {
        QList<QFileInfo> possibleFiles;
        foreach (const QString &dir, directories) {
            QFileInfo candidate(dir + QLatin1Char('/') + filePath);
            if (candidate.exists()
                && !possibleFiles.contains(candidate)) {
                possibleFiles << candidate;
            }
        }
        if (possibleFiles.size() == 1)
            editable.file = Utils::FileName(possibleFiles.first());
        // Let the Makestep apply additional heuristics (based on
        // files in ther project) if we cannot uniquely
        // identify the file!
    }

    IOutputParser::taskAdded(editable, linkedLines, skippedLines);
}
