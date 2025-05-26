// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
