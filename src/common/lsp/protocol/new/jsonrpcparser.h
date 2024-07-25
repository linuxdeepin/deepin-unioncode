// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSONRPCPARSER_H
#define JSONRPCPARSER_H

#include <QThread>

namespace newlsp {
class JsonRpcParser : public QObject
{
    Q_OBJECT
public:
    JsonRpcParser(QObject *parent = nullptr) : QObject(parent) {}

public Q_SLOTS:
    void doParseReadLine(const QByteArray &line);

Q_SIGNALS:
    void readedJsonObject(const QJsonObject &obj);
};

}
#endif // JSONRPCPARSER_H
