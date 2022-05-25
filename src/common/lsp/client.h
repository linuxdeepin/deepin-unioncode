#ifndef CLIENT_H
#define CLIENT_H

#include "common/lsp/protocol.h"

#include <QTcpSocket>
#include <QThread>

struct Head
{
    QString workspace;
    QString language;

    Head();
    Head(const QString &workspace, const QString &language);
    Head(const Head &head);
    bool isValid() const;
};

namespace lsp {

class ClientPrivate;
class Client : public QTcpSocket
{
    Q_OBJECT
    friend class ClientManager;
    ClientPrivate *const d;
    void initRequest(const Head &head, const QString &compile); // yes
    void shutdownRequest();
    void exitRequest();
public:
    explicit Client(QObject *parent = nullptr);
    virtual ~Client();
    static bool exists(const QString &progrma);
    void openRequest(const QString &filePath); // yes
    void closeRequest(const QString &filePath); // yes
    void changeRequest(const QString &filePath, const QByteArray &text); // yes
    void symbolRequest(const QString &filePath); // yes
    void renameRequest(const QString &filePath, const Position &pos, const QString &newName); //yes
    void definitionRequest(const QString &filePath, const Position &pos); // yes
    void completionRequest(const QString &filePath, const Position &pos); // yes
    void signatureHelpRequest(const QString &filePath, const Position &pos); // yes
    void referencesRequest(const QString &filePath, const Position &pos);
    void docHighlightRequest(const QString &filePath, const Position &pos);
    void docSemanticTokensFull(const QString &filePath); //yes
    void docHoverRequest(const QString &filePath, const Position &pos); // yes
    void processJson(const QJsonObject &jsonObj);

    lsp::SemanticTokensProvider initSecTokensProvider();

signals:
    void request();
    void notification(const QString &jsonStr);
    void notification(const DiagnosticsParams &diagnostics);
    void requestResult(const SemanticTokensProvider &tokensProvider);
    void requestResult(const Symbols &symbols);
    void requestResult(const Locations &locations);
    void requestResult(const CompletionProvider &completionProvider);
    void requestResult(const SignatureHelps &signatureHelps);
    void requestResult(const Hover &hover);
    void requestResult(const Highlights &highlights);
    void requestResult(const DefinitionProvider &definitionProvider);
    void requestResult(const QList<Data> &tokensResult);
    void requestResult(const RenameChanges &changes);
    void requestResult(const References &refs);

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

class ClientReadThreadPrivate;
class ClientReadThread : public QThread
{
    Q_OBJECT
    ClientReadThreadPrivate *const d;
public:
    ClientReadThread(Client *client);
    virtual void stop();
    virtual void run();
};

class ClientManager final
{
    Q_DISABLE_COPY(ClientManager)
public:
    ClientManager() {}

    static ClientManager *instance();

    void initClient(const Head &head, const QString &complie);

    void shutdownClient(const Head &head);

    Client *get(const Head &head);

private:
    QHash<Head, Client*> clients;
};

} // namespace lsp

uint qHash(const Head &key, uint seed = 0);
bool operator == (const Head &t1, const Head &t2);

#endif // CLIENT_H
