// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IOUTPUTPARSER_H
#define IOUTPUTPARSER_H

#include "task.h"
#include "common/widget/outputpane.h"

#include <QObject>

class Task;
class IOutputParser : public QObject
{
    Q_OBJECT
public:
    IOutputParser() = default;
    ~IOutputParser() override;

    virtual void appendOutputParser(IOutputParser *parser);

    IOutputParser *takeOutputParserChain();

    IOutputParser *childParser() const;
    void setChildParser(IOutputParser *parser);

    virtual void stdOutput(const QString &line, OutputPane::OutputFormat format);
    virtual void stdError(const QString &line);

    virtual bool hasFatalErrors() const;
    virtual void setWorkingDirectory(const QString &workingDirectory);

    void flush(); // flush out pending tasks

    static QString rightTrimmed(const QString &in);

signals:
    void addOutput(const QString &string, OutputPane::OutputFormat format);
    void addTask(const Task &task, int linkedOutputLines = 0, int skipLines = 0);

public slots:
    virtual void outputAdded(const QString &string, OutputPane::OutputFormat format);
    virtual void taskAdded(const Task &task, int linkedOutputLines = 0, int skipLines = 0);

private:
    virtual void doFlush();

    IOutputParser *outParser = nullptr;
};

#endif // IOUTPUTPARSER_H
