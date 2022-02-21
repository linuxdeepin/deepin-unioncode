#ifndef CLIENT_H
#define CLIENT_H

#include "common/lsp/protocol.h"

#include <QProcess>
#include <QThread>

namespace lsp {

class ClientPrivate;
class Client : public QProcess
{
    Q_OBJECT
    friend class ReadThread;
    ClientPrivate *const d;
public:
    explicit Client(QObject *parent = nullptr);
    virtual ~Client();
    static bool exists(const QString &progrma);
    void initRequest(const QString &rootPath); // yes
    void openRequest(const QString &filePath); // yes
    void closeRequest(const QString &filePath); // yes
    void changeRequest(const QString &filePath, const QByteArray &text); // yes
    void symbolRequest(const QString &filePath); // yes
    void definitionRequest(const QString &filePath, const Position &pos); // yes
    void completionRequest(const QString &filePath, const Position &pos); // yes
    void signatureHelpRequest(const QString &filePath, const Position &pos); // yes
    void referencesRequest(const QString &filePath, const Position &pos);
    void docHighlightRequest(const QString &filePath, const Position &pos);
    void docSemanticTokensFull(const QString &filePath); //yes
    void docHoverRequest(const QString &filePath, const Position &pos); // yes
    void shutdownRequest();
    void exitRequest();
    void processJson(const QJsonObject &jsonObj);

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

private:
    bool calledError(const QJsonObject &jsonObj);
    bool calledResult(const QJsonObject &jsonObj); //found result key from json && not found method
    bool initResult(const QJsonObject &jsonObj); // client call server rpc return
    bool openResult(const QJsonObject &jsonObj); // client call server rpc return
    bool changeResult(const QJsonObject &jsonObj); // client call server rpc return
    bool symbolResult(const QJsonObject &jsonObj); // client call server rpc return
    bool definitionResult(const QJsonObject &jsonObj); // client call server rpc return
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
    void readForThread();
    QStringList cvtStringList(const QJsonArray &array);
};

} // namespace lsp
#endif // CLIENT_H
