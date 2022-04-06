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
#ifndef CMAKEPARSER_H
#define CMAKEPARSER_H

#include "ioutputparser.h"
#include "task.h"

#include <QObject>
#include <QRegularExpression>

class CMakeParser : public IOutputParser
{
    Q_OBJECT
public:
    explicit CMakeParser();
    void stdError(const QString &line) override;

protected:
    void doFlush() override;

private:
    enum TripleLineError { NONE, LINE_LOCATION, LINE_DESCRIPTION, LINE_DESCRIPTION2 };

    TripleLineError expectTripleLineErrorData = NONE;

    Task lastTask;
    QRegExp commonError;
    QRegExp nextSubError;
    QRegularExpression locationLine;
    bool skippedFirstEmptyLine = false;
    int lines = 0;
};

#endif // CMAKEPARSER_H
