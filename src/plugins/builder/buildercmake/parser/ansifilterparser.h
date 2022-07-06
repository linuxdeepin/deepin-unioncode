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
#ifndef ANSIFILTERPARSER_H
#define ANSIFILTERPARSER_H

#include "services/builder/ioutputparser.h"

class AnsiFilterParser : public IOutputParser
{
    Q_OBJECT

public:
    AnsiFilterParser();
    void stdOutput(const QString &line) override;
    void stdError(const QString &line) override;

private:
    QString filterLine(const QString &line);
};

#endif // ANSIFILTERPARSER_H
