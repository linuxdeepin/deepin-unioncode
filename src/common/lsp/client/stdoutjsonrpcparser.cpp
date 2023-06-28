// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stdoutjsonrpcparser.h"
#include "common/lsp/protocol/new/jsonrpcparser.h"

newlsp::StdoutJsonRpcParser::StdoutJsonRpcParser(QObject *parent)
    : QObject(parent)
    , d (new JsonRpcParser)
{
    QObject::connect(d, &JsonRpcParser::readedJsonObject,
                     this, &StdoutJsonRpcParser::readedJsonObject,
                     Qt::DirectConnection);
}

newlsp::StdoutJsonRpcParser::~StdoutJsonRpcParser()
{
    if (d) {
        delete d;
    }
}

void newlsp::StdoutJsonRpcParser::doReadedLine(const QByteArray &line)
{
    d->doParseReadLine(line);
}
