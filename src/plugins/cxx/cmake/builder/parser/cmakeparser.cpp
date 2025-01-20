// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakeparser.h"

#include "common/util/fileutils.h"
#include "common/util/qtcassert.h"
#include "services/project/projectservice.h"

const char COMMON_ERROR_PATTERN[] = "^CMake Error at (.*):([0-9]*)( \\((.*)\\))?:";
const char NEXT_SUBERROR_PATTERN[] = "^CMake Error in (.*):";
const char LOCATION_LINE_PATTERN[] = ":(\\d+):(?:(\\d+))?$";

const char TASK_CATEGORY_BUILDSYSTEM[] = "Task.Category.Buildsystem";

QString assembleCmakePath(const QString &relativePath)
{
    auto prjService = dpfGetService(dpfservice::ProjectService);
    QString workSpace = prjService->getActiveProjectInfo().workspaceFolder();
    return workSpace + QDir::separator() + relativePath;
}

CMakeParser::CMakeParser()
{
    commonError.setPattern(QLatin1String(COMMON_ERROR_PATTERN));
    QTC_CHECK(commonError.isValid());

    nextSubError.setPattern(QLatin1String(NEXT_SUBERROR_PATTERN));
    QTC_CHECK(nextSubError.isValid());

    locationLine.setPattern(QLatin1String(LOCATION_LINE_PATTERN));
    QTC_CHECK(locationLine.isValid());
}

void CMakeParser::stdOutput(const QString &line, OutputPane::OutputFormat format)
{
    emit outputAdded(line, format);
    AbstractOutputParser::stdOutput(line, format);
}

void CMakeParser::stdError(const QString &line)
{
    QString trimmedLine = rightTrimmed(line);

    auto commonErrorMatch = commonError.match(trimmedLine);
    auto nextSubErrorMatch = nextSubError.match(trimmedLine);
    switch (expectTripleLineErrorData) {
    case NONE:
        if (trimmedLine.isEmpty() && !lastTask.isNull()) {
            if (skippedFirstEmptyLine)
                doFlush();
            else
                skippedFirstEmptyLine = true;
            return;
        }
        skippedFirstEmptyLine = false;

        if (commonErrorMatch.hasMatch()) {
            lastTask = Task(Task::Error, QString(), Utils::FileName::fromUserInput(assembleCmakePath(commonErrorMatch.captured(1))),
                              commonErrorMatch.captured(2).toInt(), TASK_CATEGORY_BUILDSYSTEM);
            lines = 1;
            return;
        }  else if (trimmedLine.startsWith(QLatin1String("  ")) && !lastTask.isNull()) {
            if (!lastTask.description.isEmpty())
                lastTask.description.append(QLatin1Char(' '));
            lastTask.description.append(trimmedLine.trimmed());
            ++lines;
            return;
        } else if (trimmedLine.endsWith(QLatin1String("in cmake code at"))) {
            expectTripleLineErrorData = LINE_LOCATION;
            doFlush();
            lastTask = Task(trimmedLine.contains(QLatin1String("Error")) ? Task::Error : Task::Warning,
                              QString(), Utils::FileName(), -1, TASK_CATEGORY_BUILDSYSTEM);
            return;
        } else if (trimmedLine.startsWith("CMake Error")) {
            lastTask = Task(Task::Error, trimmedLine,
                              Utils::FileName(), -1, TASK_CATEGORY_BUILDSYSTEM);
            lines = 1;
            return;
        } else if (nextSubErrorMatch.hasMatch()) {
            lastTask = Task(Task::Error, QString(), Utils::FileName::fromUserInput(assembleCmakePath(nextSubErrorMatch.captured(1))), -1,
                              TASK_CATEGORY_BUILDSYSTEM);
            lines = 1;
            return;
        }

        AbstractOutputParser::stdError(line);
        return;
    case LINE_LOCATION:
        {
            QRegularExpressionMatch m = locationLine.match(trimmedLine);
            QTC_CHECK(m.hasMatch());
            lastTask.file = Utils::FileName::fromUserInput(trimmedLine.mid(0, m.capturedStart()));
            lastTask.line = m.captured(1).toInt();
            expectTripleLineErrorData = LINE_DESCRIPTION;
        }
        return;
    case LINE_DESCRIPTION:
        lastTask.description = trimmedLine;
        if (trimmedLine.endsWith(QLatin1Char('\"')))
            expectTripleLineErrorData = LINE_DESCRIPTION2;
        else {
            expectTripleLineErrorData = NONE;
            doFlush();
        }
        return;
    case LINE_DESCRIPTION2:
        lastTask.description.append(QLatin1Char('\n'));
        lastTask.description.append(trimmedLine);
        expectTripleLineErrorData = NONE;
        doFlush();
        return;
    }
}

void CMakeParser::doFlush()
{
    if (lastTask.isNull())
        return;
    Task t = lastTask;
    lastTask.clear();
    emit addTask(t, lines, 1);
    lines = 0;
}
