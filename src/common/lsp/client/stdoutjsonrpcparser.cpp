// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stdoutjsonrpcparser.h"
#include "common/lsp/protocol/new/jsonrpcparser.h"

#include <QDebug>
#include <QJsonDocument>
#include <QRegularExpression>

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

    QRegularExpression regExpContentLength("^Content-Length:\\s?(?<Length>[0-9]+)");
    auto match = regExpContentLength.match(line);
    if (match.hasMatch())
        contentLength = match.captured("Length").toInt();

    // The data in `line` may be truncated and assembled according to the `Content-Length`
    if (!outputCache.isEmpty() || (contentLength != 0 && data.contains("\"jsonrpc\":") && !checkJsonValid(data))) {
        outputCache.append(data.mid(0, contentLength - outputCache.size()));
        if (outputCache.size() == contentLength && checkJsonValid(outputCache)) {
            data = outputCache;
            outputCache.clear();
            contentLength = 0;
        } else if (outputCache.size() >= contentLength) {
            outputCache.clear();
            contentLength = 0;
        } else {
            return;
        }
    }

    d->doParseReadLine(data);
}
