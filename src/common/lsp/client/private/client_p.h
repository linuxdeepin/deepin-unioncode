// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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

    struct RequestInfo
    {
        QString method;
        QString file;
    };

    friend class Client;
    Client *const q;
    int requestIndex;
    QHash<int, RequestInfo> requestSave;
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
    bool rangeFormattingResult(const QJsonObject &jsonObj); // client call server rpc return
    bool signatureHelpResult(const QJsonObject &jsonObj); // client call server rpc return
    bool hoverResult(const QJsonObject &jsonObj); // client call server rpc return
    bool referencesResult(const QJsonObject &jsonObj); // client call server rpc return
    bool docHighlightResult(const QJsonObject &jsonObj); // client call server rpc return
    bool docSemanticTokensFullResult(const QJsonObject &jsonObj); // client call server rpc return
    bool closeResult(const QJsonObject &jsonObj); // client call server rpc return
    bool shutdownResult(const QJsonObject &jsonObj);
    bool exitResult(const QJsonObject &jsonObj);
    bool switchHeaderSourceResult(const QJsonObject &jsonObj);

    /* server called method */
    bool serverCalled(const QJsonObject &jsonObj); // not found result key from json && found key method
    bool diagnosticsCalled(const QJsonObject &jsonObj);

    QList<DocumentSymbol> parseDocumentSymbol(const QJsonArray &array);
    QList<SymbolInformation> parseDocumentSymbolInfo(const QJsonArray &array);
    QList<DocumentHighlight> parseDocumentHighlight(const QJsonArray &array);
    Range parseRange(const QJsonObject &obj);
    Location parseLocation(const QJsonObject &obj);

public Q_SLOTS:
    // readed parse
    void doReadStdoutLine();
    void identifyJsonObject(const QJsonObject &jsonObj);

    void callMethod(const QString &method, const QJsonObject &params, const QString &filePath = "");
    void callNotification(const QString &method, const QJsonObject &params);
    void writeLspData(const QByteArray &jsonObj);
};

}

#endif // CLIENT_P_H
