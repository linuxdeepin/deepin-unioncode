// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stdoutjsonrpcparser.h"
#include "common/lsp/protocol/new/jsonrpcparser.h"

#include <QDebug>
#include <QJsonDocument>

newlsp::StdoutJsonRpcParser::StdoutJsonRpcParser(QObject *parent)
    : QObject(parent), d(new JsonRpcParser)
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

bool newlsp::StdoutJsonRpcParser::checkJsonValid(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument::fromJson(data, &error);
    return error.error == QJsonParseError::NoError;
}

void newlsp::StdoutJsonRpcParser::doReadedLine(const QByteArray &line)
{
    auto data = line;
    if (!outputCache.isEmpty() || (data.contains("\"jsonrpc\":") && !checkJsonValid(data))) {
        outputCache.append(data);
        if (!checkJsonValid(outputCache))
            return;

        data = outputCache;
        outputCache.clear();
    }
    d->doParseReadLine(data);
}
