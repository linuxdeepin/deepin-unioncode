// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jsonrpcparser.h"
#include "basicjsonstructures.h"

#include <QRegularExpression>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>

#include <iostream>

namespace newlsp {

void JsonRpcParser::doParseReadLine(const QByteArray &line)
{
    static QVector<bool> formatFlag = {false, false, false, false};
    static int contentLengthVal{0};
    static QString contentTypeVal{""};
    static QString charsetVal{""};

     if (qAppName() == "unioncode") {
         qInfo() << line;
     }

    auto cleanCache = [=]() {
        contentLengthVal = 0;
        contentTypeVal = "";
        charsetVal = "";
        formatFlag = {false, false, false, false};
    };

    QRegularExpression regExpContentLength("^Content-Length:\\s?(?<" + newlsp::RK_CONTENT_LENGTH + ">[0-9]+)");
    auto match = regExpContentLength.match(line);
    if (!formatFlag[0] && match.hasMatch()) {
        formatFlag[0] = true;
        contentLengthVal = match.captured(newlsp::RK_CONTENT_LENGTH).toInt();
        return;
    }

    QRegularExpression regExpContentType("^Content-Type:\\s?(?<" + newlsp::RK_CONTENT_TYPE + ">\\S+);" +
                                         "\\s?charset=(?<" + newlsp::RK_CHARSET + ">\\S+)");
    match = regExpContentType.match(line);
    if (!formatFlag[1] && formatFlag[0] && match.hasMatch()) {
        formatFlag[1] = true;
        contentTypeVal = match.captured(newlsp::RK_CONTENT_TYPE);
        charsetVal = match.captured(newlsp::RK_CHARSET);
        return;
    }

    QRegularExpression regExpSplitter("");
    match = regExpSplitter.match(line);
    if (!formatFlag[2] && formatFlag[0] && match.hasMatch()) {
        formatFlag[2] = true;
        return;
    }

    if ((formatFlag[0] && formatFlag[2])
            || (formatFlag[0] && formatFlag[1] && formatFlag[2])) {
        if (contentLengthVal > line.size()) {
            std::cout << "json size error" << " "
                      << "ContentLength: " << " "
                      << contentLengthVal << " "
                      << "Json size:" << " "
                      << line.size() << " "
                      << QString(line).toStdString() << " "
                      << std::endl;
        } else {
            QByteArray other = line.mid(contentLengthVal, line.size());
            QByteArray body = line.mid(0, contentLengthVal);
            QJsonParseError err;
            QJsonObject jsonObj = QJsonDocument::fromJson(body, &err).object();
            if (err.error != QJsonParseError::NoError) {
                std::cerr << err.errorString().toStdString() << std::endl;
            } else {
                Q_EMIT readedJsonObject(jsonObj);
                cleanCache();
                if (!other.isEmpty()) {
                    // if (qAppName() == "unioncode") {
                    //     qInfo() <<  "do next" << other;
                    // }
                    return doParseReadLine(other);
                }
            }
        }
    }
    cleanCache();
}

} //newlsp
