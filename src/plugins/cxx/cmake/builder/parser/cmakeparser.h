// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEPARSER_H
#define CMAKEPARSER_H

#include "base/abstractoutputparser.h"
#include "common/type/task.h"

#include <QObject>
#include <QRegularExpression>

class CMakeParser : public AbstractOutputParser
{
    Q_OBJECT
public:
    explicit CMakeParser();
    virtual void stdOutput(const QString &line, OutputPane::OutputFormat format) override;
    void stdError(const QString &line) override;

protected:
    void doFlush() override;

private:
    enum TripleLineError { NONE, LINE_LOCATION, LINE_DESCRIPTION, LINE_DESCRIPTION2 };

    TripleLineError expectTripleLineErrorData = NONE;

    Task lastTask;
    QRegularExpression commonError;
    QRegularExpression nextSubError;
    QRegularExpression locationLine;
    bool skippedFirstEmptyLine = false;
    int lines = 0;
};

#endif // CMAKEPARSER_H
