/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<huangyub@uniontech.com>
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
#ifndef IOUTPUTPARSER_H
#define IOUTPUTPARSER_H

#include "buildstep.h"

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

    virtual void stdOutput(const QString &line);
    virtual void stdError(const QString &line);

    virtual bool hasFatalErrors() const;
    virtual void setWorkingDirectory(const QString &workingDirectory);

    void flush(); // flush out pending tasks

    static QString rightTrimmed(const QString &in);

signals:
    void addOutput(const QString &string, OutputFormat format);
    void addTask(const Task &task, int linkedOutputLines = 0, int skipLines = 0);

public slots:
    virtual void outputAdded(const QString &string, OutputFormat format);
    virtual void taskAdded(const Task &task, int linkedOutputLines = 0, int skipLines = 0);

private:
    virtual void doFlush();

    IOutputParser *outParser = nullptr;
};

#endif // IOUTPUTPARSER_H
