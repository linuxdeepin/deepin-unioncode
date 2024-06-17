// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTOUTPUTPARSER_H
#define ABSTRACTOUTPUTPARSER_H

#include "common/type/task.h"
#include "common/widget/outputpane.h"

#include <QObject>
#include <memory> // For using std::unique_ptr

class Task;

class AbstractOutputParser : public QObject
{
    Q_OBJECT
public:
    AbstractOutputParser(QObject *parent = nullptr);
    virtual ~AbstractOutputParser() = default; // Use default destructor for unique_ptr

    // Chain management methods
    virtual void appendOutputParser(AbstractOutputParser *parser);
    AbstractOutputParser *takeOutputParserChain();
    AbstractOutputParser *childParser() const;
    void setChildParser(AbstractOutputParser *parser);

    // Output handling methods (virtual for customization in derived classes)
    virtual void stdOutput(const QString &line, OutputPane::OutputFormat format);
    virtual void stdError(const QString &line);

    // Error checking and working directory setting
    virtual bool hasFatalErrors() const;
    virtual void setWorkingDirectory(const QString &workingDirectory);

    // Method to flush pending tasks
    void flush();

    // Utility method to trim whitespace from the right side of a string
    static QString rightTrimmed(const QString &in); 

signals:
    // Signals for adding output and tasks
    void addOutput(const QString &string, OutputPane::OutputFormat format);
    void addTask(const Task &task, int linkedOutputLines = 0, int skipLines = 0);

public slots:
    // Slots for handling added output and tasks
    virtual void outputAdded(const QString &string, OutputPane::OutputFormat format);
    virtual void taskAdded(const Task &task, int linkedOutputLines = 0, int skipLines = 0);

private:
    virtual void doFlush();

    class AbstractOutputParserPrivate;  // Forward declare private implementation class
    std::unique_ptr<AbstractOutputParserPrivate> d; // Use unique_ptr for memory management
};

#endif // ABSTRACTOUTPUTPARSER_H
