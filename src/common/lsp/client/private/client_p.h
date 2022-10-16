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
#ifndef CLIENT_P_H
#define CLIENT_P_H

#include "common/lsp/client/stdoutjsonrpcparser.h"
#include "common/lsp/protocol/newprotocol.h"
#include "common/lsp/protocol/protocol.h"

#include <QHash>

namespace newlsp {

class Client;
class ClientPrivate : public newlsp::StdoutJsonRpcParser
{
    Q_OBJECT
    friend class Client;
    Client *const q;
    int requestIndex;
    QHash<int, QString> requestSave;
    int semanticTokenResultId;
    QHash<QString, int> fileVersion;
    lsp::SemanticTokensProvider secTokensProvider;
    newlsp::ProjectKey proKey;

    ClientPrivate(Client *const q);

    QStringList cvtStringList(const QJsonArray &array);

    /* server response parse */
    bool calledError(const QJsonObject &jsonObj);
    bool calledResult(const QJsonObject &jsonObj); //found result key from json && not found method
    bool initResult(const QJsonObject &jsonObj); // client call server rpc return
    bool openResult(const QJsonObject &jsonObj); // client call server rpc return
    bool changeResult(const QJsonObject &jsonObj); // client call server rpc return
    bool symbolResult(const QJsonObject &jsonObj); // client call server rpc return
    bool renameResult(const QJsonObject &jsonObj); // client call server rpc return
    bool definitionResult(const QJsonObject &jsonObj); // client call server rpc return above uri
    bool completionResult(const QJsonObject &jsonObj); // client call server rpc return
    bool signatureHelpResult(const QJsonObject &jsonObj); // client call server rpc return
    bool hoverResult(const QJsonObject &jsonObj); // client call server rpc return
    bool referencesResult(const QJsonObject &jsonObj); // client call server rpc return
    bool docHighlightResult(const QJsonObject &jsonObj); // client call server rpc return
    bool docSemanticTokensFullResult(const QJsonObject &jsonObj); // client call server rpc return
    bool closeResult(const QJsonObject &jsonObj); // client call server rpc return
    bool shutdownResult(const QJsonObject &jsonObj);
    bool exitResult(const QJsonObject &jsonObj);

    /* server called method */
    bool serverCalled(const QJsonObject &jsonObj); // not found result key from json && found key method
    bool diagnosticsCalled(const QJsonObject &jsonObj);

public Q_SLOTS:
    // readed parse
    void doReadStdoutLine();
    void identifyJsonObject(const QJsonObject &jsonObj);

    void callMethod(const QString &method, const QJsonObject &params);
    void callNotification(const QString &method, const QJsonObject &params);
    void writeLspData(const QByteArray &jsonObj);
};

}

#endif // CLIENT_P_H
