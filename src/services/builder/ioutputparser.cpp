// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ioutputparser.h"

IOutputParser::~IOutputParser()
{
    delete outParser;
}

void IOutputParser::appendOutputParser(IOutputParser *parser)
{
    if (!parser)
        return;
    if (outParser) {
        outParser->appendOutputParser(parser);
        return;
    }

    outParser = parser;
    connect(parser, &IOutputParser::addOutput,
            this, &IOutputParser::outputAdded, Qt::DirectConnection);
    connect(parser, &IOutputParser::addTask,
            this, &IOutputParser::taskAdded, Qt::DirectConnection);
}

IOutputParser *IOutputParser::takeOutputParserChain()
{
    IOutputParser *parser = outParser;
    disconnect(parser, &IOutputParser::addOutput, this, &IOutputParser::outputAdded);
    disconnect(parser, &IOutputParser::addTask, this, &IOutputParser::taskAdded);
    outParser = nullptr;
    return parser;
}

IOutputParser *IOutputParser::childParser() const
{
    return outParser;
}

void IOutputParser::setChildParser(IOutputParser *parser)
{
    if (outParser != parser)
        delete outParser;
    outParser = parser;
    if (parser) {
        connect(parser, &IOutputParser::addOutput,
                this, &IOutputParser::outputAdded, Qt::DirectConnection);
        connect(parser, &IOutputParser::addTask,
                this, &IOutputParser::taskAdded, Qt::DirectConnection);
    }
}

void IOutputParser::stdOutput(const QString &line, OutputPane::OutputFormat format)
{
    if (outParser)
        outParser->stdOutput(line, format);
}

void IOutputParser::stdError(const QString &line)
{
    if (outParser)
        outParser->stdError(line);
}

void IOutputParser::outputAdded(const QString &string, OutputPane::OutputFormat format)
{
    emit addOutput(string, format);
}

void IOutputParser::taskAdded(const Task &task, int linkedOutputLines, int skipLines)
{
    emit addTask(task, linkedOutputLines, skipLines);
}

void IOutputParser::doFlush()
{ }

bool IOutputParser::hasFatalErrors() const
{
    return outParser && outParser->hasFatalErrors();
}

void IOutputParser::setWorkingDirectory(const QString &workingDirectory)
{
    if (outParser)
        outParser->setWorkingDirectory(workingDirectory);
}

void IOutputParser::flush()
{
    doFlush();
    if (outParser)
        outParser->flush();
}

QString IOutputParser::rightTrimmed(const QString &in)
{
    int pos = in.length();
    for (; pos > 0; --pos) {
        if (!in.at(pos - 1).isSpace())
            break;
    }
    return in.mid(0, pos);
}
