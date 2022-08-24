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
#include "serverhandler.h"
#include "tools.h"

#include <jsonrpccpp/server/connectors/tcpsocketserver.h>

#include <QApplication>

#include <iostream>

namespace OptionNames
{
const QString port {"port"};
}

namespace DefaultValues
{
const QString port{"3309"};
}

const QList<QCommandLineOption> options
{
    {
        QCommandLineOption {
            OptionNames::port, QString("Server open port, default %0.\n").arg(DefaultValues::port), "number", DefaultValues::port
        }
    },
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;

    parser.addOptions(options);
    parser.addHelpOption();
    parser.process(a);

    auto list = parser.optionNames();

    quint16 port = DefaultValues::port.toUShort();
    if (list.contains(OptionNames::port)) {
        port = parser.value(OptionNames::port).toUShort();
    }

    jsonrpc::TcpSocketServer server("127.0.0.1", port);
    ServerHandler hand(server, new Tools);

    if (hand.StartListening()) {
        std::cout << "Server started successfully name: "
                  << QCoreApplication::applicationName().toStdString()
                  << "port: " << port
                  << std::endl;
    } else {
        std::cout << "Error starting Server name: "
                  << QCoreApplication::applicationName().toStdString()
                  << "port: " << port
                  << std::endl;
    }

    return a.exec();
}
