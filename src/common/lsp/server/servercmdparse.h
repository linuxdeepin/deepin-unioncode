/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef SERVERCMDPARSE_H
#define SERVERCMDPARSE_H

#include <QCoreApplication>
#include <QCommandLineParser>

namespace newlsp {
extern const QString port;
extern const QString mode;
extern const QString parentPid;
extern const QString stdio;
extern const QString tcp;
extern const QString portDefault;
extern const QString parentPidDefault;
class ServerCmdParsePrivate;
class ServerCmdParse : QCommandLineParser
{
    ServerCmdParsePrivate *const d;
public:
    ServerCmdParse(const QCoreApplication &app);
    virtual ~ServerCmdParse();
    std::optional<uint> parentPid() const;
    std::optional<std::string> mode() const;
    std::optional<uint> port() const;
    virtual void bindOptions();
};

}
#endif // SERVERCMDPARSE_H
