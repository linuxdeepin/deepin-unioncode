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
#ifndef CLIENT_H
#define CLIENT_H

#include "common/lsp/protocol.h"
#include "common/lsp/newprotocol.h"

#include <QTcpSocket>
#include <QThread>

namespace lsp {

struct Head
{
    QString workspace;
    QString language;

    Head();
    Head(const QString &workspace, const QString &language);
    Head(const Head &head);
    bool isValid() const;
};

class ClientPrivate;
class Client : public QObject
{
    Q_OBJECT
    ClientPrivate *const d;
public:
    explicit Client(unsigned int port, const QString &host = "127.0.0.1");
    virtual ~Client();

    lsp::SemanticTokensProvider initSecTokensProvider();
    static bool exists(const QString &progrma);

public slots:
    void setHead(const lsp::Head &head);
    void initRequest(const QString &compile); // yes
    void shutdownRequest();
    void exitRequest();
    void openRequest(const QString &filePath); // yes
    void closeRequest(const QString &filePath); // yes
    void changeRequest(const QString &filePath, const QByteArray &text); // yes
    void symbolRequest(const QString &filePath); // yes
    void renameRequest(const QString &filePath, const lsp::Position &pos, const QString &newName); //yes
    void definitionRequest(const QString &filePath, const lsp::Position &pos); // yes
    void completionRequest(const QString &filePath, const lsp::Position &pos); // yes
    void signatureHelpRequest(const QString &filePath, const lsp::Position &pos); // yes
    void referencesRequest(const QString &filePath, const lsp::Position &pos);
    void docHighlightRequest(const QString &filePath, const lsp::Position &pos);
    void docSemanticTokensFull(const QString &filePath); //yes
    void docHoverRequest(const QString &filePath, const lsp::Position &pos); // yes
    void processJson(const QJsonObject &jsonObj);

signals:
    void request();
    void notification(const QString &jsonStr);
    void notification(const lsp::DiagnosticsParams &diagnostics);
    void requestResult(const lsp::SemanticTokensProvider &tokensProvider);
    void requestResult(const lsp::Symbols &symbols);
    void requestResult(const lsp::Locations &locations);
    void requestResult(const lsp::CompletionProvider &completionProvider);
    void requestResult(const lsp::SignatureHelps &signatureHelps);
    void requestResult(const lsp::Hover &hover);
    void requestResult(const lsp::Highlights &highlights);
    void requestResult(const lsp::DefinitionProvider &definitionProvider);
    void requestResult(const QList<lsp::Data> &tokensResult);
    void requestResult(const newlsp::Workspace::WorkspaceEdit &changes);
    void requestResult(const lsp::References &refs);

private:
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

    bool serverCalled(const QJsonObject &jsonObj); // not found result key from json && found key method
    bool diagnostics(const QJsonObject &jsonObj);

private slots:
    QStringList cvtStringList(const QJsonArray &array);
};

uint qHash(const Head &key, uint seed = 0);
bool operator == (const Head &t1, const Head &t2);

} // namespace lsp

#endif // CLIENT_H
