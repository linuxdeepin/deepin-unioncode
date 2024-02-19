// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

public Q_SLOTS:
    void doReadedLine(const QByteArray &line);

Q_SIGNALS:
    void readedJsonObject(const QJsonObject &);

private:
    QByteArray outputCache;
};

}

#endif // STDOUTJSONRPCPARSER_H
