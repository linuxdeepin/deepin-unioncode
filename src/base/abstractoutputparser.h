// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTOUTPUTPARSER_H
#define ABSTRACTOUTPUTPARSER_H

#include "common/type/task.h"
#include "common/widget/outputpane.h"

#include <QObject>

class Task;
class AbstractOutputParser : public QObject
{
    Q_OBJECT
public:
    AbstractOutputParser() = default;
    ~AbstractOutputParser() override;

    virtual void appendOutputParser(AbstractOutputParser *parser);

    AbstractOutputParser *takeOutputParserChain();

    AbstractOutputParser *childParser() const;
    void setChildParser(AbstractOutputParser *parser);

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

    AbstractOutputParser *outParser = nullptr;
};

#endif // ABSTRACTOUTPUTPARSER_H
