// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef STDOUTJSONRPCPARSER_H
#define STDOUTJSONRPCPARSER_H

#include <QThread>

namespace newlsp {

class JsonRpcParser;
class StdoutJsonRpcParser : public QObject
{
    Q_OBJECT
    JsonRpcParser *const d;
public:
    explicit StdoutJsonRpcParser(QObject *parent = nullptr);
    virtual ~StdoutJsonRpcParser();

    bool checkJsonValid(const QByteArray &data);

public Q_SLOTS:
    void doReadedLine(const QByteArray &line);

Q_SIGNALS:
    void readedJsonObject(const QJsonObject &);

private:
    QByteArray outputCache;
    int contentLength = 0;
};

}

#endif // STDOUTJSONRPCPARSER_H
