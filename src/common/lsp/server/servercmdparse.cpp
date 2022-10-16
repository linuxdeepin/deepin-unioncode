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
#include "servercmdparse.h"

namespace newlsp {

const QString port {"port"};
const QString mode {"mode"};
const QString parentPid {"parentPid"};
const QString stdio {"stdio"};
const QString tcp {"tcp"};
const QString portDefault {"3307"};
const QString parentPidDefault {"0"};

class ServerCmdParsePrivate
{
    friend class ServerCmdParse;
    QList<QCommandLineOption> options
    {
        {
            parentPid,
                    "Server use timer to watch parent process id, "
                    "if process id no exist, well quit this program.\n",
                    parentPidDefault
        },{
            mode,
                    "Server user Server startup mode,can use "
                    "[" + stdio + "] ["+ tcp +"], at default is stdio",
                    stdio
        },{
            port,
                    "The parameter passed during startup is valid only when the mode is tcp",
                    portDefault
        }
    };
};

ServerCmdParse::ServerCmdParse(const QCoreApplication &app)
    : QCommandLineParser ()
    , d (new ServerCmdParsePrivate())
{
    bindOptions();
    QCommandLineParser::process(app);
}

ServerCmdParse::~ServerCmdParse()
{
    if (d)
        delete d;
}

std::optional<uint> ServerCmdParse::parentPid() const
{
    std::optional<qint64> ret;
    if (optionNames().contains(newlsp::parentPid)) {
        ret = value(newlsp::parentPid).toUInt();
    }
    return ret;
}

std::optional<std::string> ServerCmdParse::mode() const
{
    std::optional<std::string> ret;
    if (optionNames().contains(newlsp::port)) {
        ret = value(newlsp::port).toStdString();
    }
    return ret;
}

std::optional<uint> ServerCmdParse::port() const
{
    std::optional<qint64> ret;
    if (optionNames().contains(newlsp::port)) {
        ret = value(newlsp::port).toUInt();
    }
    return ret;
}

void ServerCmdParse::bindOptions()
{
    QCommandLineParser::addOptions(d->options);
}

} // newlsp
