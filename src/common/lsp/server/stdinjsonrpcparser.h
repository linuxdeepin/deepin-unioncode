// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef STDINJSONRPCPARSER_H
#define STDINJSONRPCPARSER_H

#include "stdinreadloop.h"
#include "common/common_global.h"

namespace newlsp {

class JsonRpcParser;
class StdinJsonRpcParserPrivate;
class COMMON_EXPORT StdinJsonRpcParser : public StdinReadLoop
{
    Q_OBJECT
    JsonRpcParser *const d;
public:
    StdinJsonRpcParser();
    virtual ~StdinJsonRpcParser();

Q_SIGNALS:
    void readedJsonObject(const QJsonObject &);
};

}
#endif // STDINREADLOOP_H
