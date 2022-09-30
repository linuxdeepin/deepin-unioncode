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

#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonDocument>

class StdinReadLoopPrivate
{
    friend class StdinReadLoop;
    QTextStream *stream{nullptr};
    StdinReadLoopPrivate() = default;
};

StdinReadLoop::StdinReadLoop()
    : QThread()
    , d (new StdinReadLoopPrivate)
{
    d->stream = new QTextStream(stdin);
}

StdinReadLoop::~StdinReadLoop()
{
    if (d) {
        if (d->stream) {
            delete d->stream;
        }
        delete d;
    }
}

void StdinReadLoop::run()
{
    QString line;
    while (d->stream->readLineInto(&line)) {
        Q_EMIT readedLine(line);
    }
}

class StdinJsonRpcParserPrivate
{
    friend class StdinJsonRpcParser;
    QString contentLengthKey{"ContentLength"};
    QString contentTypeKey{"ContentType"};
    QString charsetKey{"charset"};
    int contentLength{0};
    QString contentType{""};
    QString charset{""};
    QVector<bool> formatFlag{false, false, false, false};
    StdinJsonRpcParserPrivate() = default;
};

StdinJsonRpcParser::StdinJsonRpcParser()
    : StdinReadLoop ()
    , d(new StdinJsonRpcParserPrivate)
{
    QObject::connect(this, &StdinReadLoop::readedLine,
                     this, &StdinJsonRpcParser::doParseReadedLine);
}

StdinJsonRpcParser::~StdinJsonRpcParser()
{
    if (d) {
        delete d;
    }
}

void StdinJsonRpcParser::clearHistory()
{
    d->contentLength = 0;
    d->contentType = "";
    d->charset = "";
    d->formatFlag = {false, false, false, false};
}

void StdinJsonRpcParser::doParseReadedLine(const QString &line)
{
    QRegularExpression regExpContentLength("^Content-Length:\\s?(?<" + d->contentLengthKey + ">[0-9]+)");
    auto match = regExpContentLength.match(line);
    if (!d->formatFlag[0] && match.hasMatch()) {
        d->formatFlag[0] = true;
        d->contentLength = match.captured(d->contentLengthKey).toInt();
        return;
    }

    QRegularExpression regExpContentType("^Content-Type:\\s?(?<" + d->contentTypeKey + ">\\S+);" +
                                         "\\s?charset=(?<" + d->charsetKey + ">\\S+)");
    match = regExpContentType.match(line);
    if (!d->formatFlag[1] && d->formatFlag[0] && match.hasMatch()) {
        d->formatFlag[1] = true;
        d->contentType = match.captured(d->contentTypeKey);
        d->charset = match.captured(d->charsetKey);
        return;
    }

    QRegularExpression regExpSplitter("");
    match = regExpSplitter.match(line);
    if (!d->formatFlag[2] && d->formatFlag[0] && match.hasMatch()) {
        d->formatFlag[2] = true;
        return;
    }

    if ((d->formatFlag[0] && d->formatFlag[2])
            || (d->formatFlag[0] && d->formatFlag[1] && d->formatFlag[2])) {
        if (d->contentLength != line.size()) {
            qCritical() << "json size error"
                        << "ContentLength: "
                        << d->contentLength
                        << "Json size: "
                        << line.size();
        } else {
            QJsonParseError err;
            QJsonObject jsonObj = QJsonDocument::fromJson(line.toUtf8(), &err).object();
            if (err.error != QJsonParseError::NoError) {
                qCritical() << err.errorString();
            } else {
                Q_EMIT readedJsonObj(jsonObj);
            }
        }
    }
    clearHistory();
}

