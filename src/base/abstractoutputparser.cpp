// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "abstractoutputparser.h"

AbstractOutputParser::~AbstractOutputParser()
{
    delete outParser;
}

void AbstractOutputParser::appendOutputParser(AbstractOutputParser *parser)
{
    if (!parser)
        return;
    if (outParser) {
        outParser->appendOutputParser(parser);
        return;
    }

    outParser = parser;
    connect(parser, &AbstractOutputParser::addOutput,
            this, &AbstractOutputParser::outputAdded, Qt::DirectConnection);
    connect(parser, &AbstractOutputParser::addTask,
            this, &AbstractOutputParser::taskAdded, Qt::DirectConnection);
}

AbstractOutputParser *AbstractOutputParser::takeOutputParserChain()
{
    AbstractOutputParser *parser = outParser;
    disconnect(parser, &AbstractOutputParser::addOutput, this, &AbstractOutputParser::outputAdded);
    disconnect(parser, &AbstractOutputParser::addTask, this, &AbstractOutputParser::taskAdded);
    outParser = nullptr;
    return parser;
}

AbstractOutputParser *AbstractOutputParser::childParser() const
{
    return outParser;
}

void AbstractOutputParser::setChildParser(AbstractOutputParser *parser)
{
    if (outParser != parser)
        delete outParser;
    outParser = parser;
    if (parser) {
        connect(parser, &AbstractOutputParser::addOutput,
                this, &AbstractOutputParser::outputAdded, Qt::DirectConnection);
        connect(parser, &AbstractOutputParser::addTask,
                this, &AbstractOutputParser::taskAdded, Qt::DirectConnection);
    }
}

void AbstractOutputParser::stdOutput(const QString &line, OutputPane::OutputFormat format)
{
    if (outParser)
        outParser->stdOutput(line, format);
}

void AbstractOutputParser::stdError(const QString &line)
{
    if (outParser)
        outParser->stdError(line);
}

void AbstractOutputParser::outputAdded(const QString &string, OutputPane::OutputFormat format)
{
    emit addOutput(string, format);
}

void AbstractOutputParser::taskAdded(const Task &task, int linkedOutputLines, int skipLines)
{
    emit addTask(task, linkedOutputLines, skipLines);
}

void AbstractOutputParser::doFlush()
{ }

bool AbstractOutputParser::hasFatalErrors() const
{
    return outParser && outParser->hasFatalErrors();
}

void AbstractOutputParser::setWorkingDirectory(const QString &workingDirectory)
{
    if (outParser)
        outParser->setWorkingDirectory(workingDirectory);
}

void AbstractOutputParser::flush()
{
    doFlush();
    if (outParser)
        outParser->flush();
}

QString AbstractOutputParser::rightTrimmed(const QString &in)
{
    int pos = in.length();
    for (; pos > 0; --pos) {
        if (!in.at(pos - 1).isSpace())
            break;
    }
    return in.mid(0, pos);
}
