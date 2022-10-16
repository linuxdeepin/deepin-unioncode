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
#include "stdinjsonrpcparser.h"
#include "common/lsp/protocol/new/jsonrpcparser.h"

#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonDocument>

#include <iostream>

namespace newlsp {

StdinJsonRpcParser::StdinJsonRpcParser()
    : StdinReadLoop ()
    , d(new JsonRpcParser())
{
    QObject::connect(this, &StdinReadLoop::readedLine,
                     d, &JsonRpcParser::doParseReadLine,
                     Qt::DirectConnection);
    QObject::connect(d, &JsonRpcParser::readedJsonObject,
                     this, &StdinJsonRpcParser::readedJsonObject,
                     Qt::DirectConnection);
}

StdinJsonRpcParser::~StdinJsonRpcParser()
{
    if (d) {
        delete d;
    }
}

} // newlsp
