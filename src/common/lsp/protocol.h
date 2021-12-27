#ifndef LANGUAGESERVERPROTOCOL_H
#define LANGUAGESERVERPROTOCOL_H

#include <QUrl>
#include <QString>
#include <QJsonValue>
#include <QProcess>

namespace lsp {

extern const QString K_ID; //int
extern const QString K_JSON_RPC; //value QString
extern const QString K_METHOD; //value QString
extern const QString K_RESULT; //value json
extern const QString K_PARAMS; //value json
extern const QString K_CAPABILITIES; //value json
extern const QString K_TEXTDOCUMENT; //value json
extern const QString K_DOCUMENTSYMBOL; //value json
extern const QString K_HIERARCHICALDOCUMENTSYMBOLSUPPORT; //value bool
extern const QString K_PUBLISHDIAGNOSTICS; //value json
extern const QString K_RELATEDINFOMATION; //value bool;
extern const QString K_INITIALIZATIONOPTIONS; // value defaule is null;
extern const QString K_PROCESSID; //value qint64 current client pid
extern const QString K_ROOTPATH; //value QString project root path
extern const QString K_ROOTURI; //value QString project root url
extern const QString K_URI; // value QString from file url
extern const QString K_VERSION; //value int
extern const QString K_LANGUAGEID; //value QString
extern const QString K_TEXT; //value QString from didOpen request document text

extern const QString H_CONTENT_LENGTH; //value int, from json length, call server request header

extern const QString V_2_0; // save QString, from K_JSON_RPC value
extern const QString V_INITIALIZE; //save QString, from K_METHOD value
extern const QString V_TEXTDOCUMENT_DIDOPEN; //save QString, from K_METHOD key
extern const QString V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS; //save QString, from K_METHOD key

extern const QString K_DIAGNOSTICS; //value is jsonArray

class Protocol final
{
    Protocol() = delete;
    Protocol(const Protocol &) = delete;
public:
    struct Position
    {
        int line;
        int character;
    };

    struct Range
    {
        Position start;
        Position end;
    };

    struct Diagnostic
    {
        QString message;
        Range range;
        int severity;
    };
    typedef QList<Diagnostic> Diagnostics;

    struct Location
    {
        Range range;
        QUrl fileUrl;
    };
    typedef QList<Location> Locations;

    struct Symbol
    {
        QString containerName;
        int kind;
        Location location;
        QString name;
    };
    typedef QList<Symbol> Symbols;

    struct CompletionItem
    {
        QString filterText;
        QString insertText;
        int insertTextFormat;
        int kind;
        int label;
        QString sortText;
    };
    typedef QList<CompletionItem> CompletionItems;

    struct SignatureHelp //暂时留空
    {

    };
    typedef QList<SignatureHelp> SignatureHelps;

    struct Hover //暂时留空
    {

    };

    struct Highlight //暂时留空
    {

    };
    typedef QList<Highlight> Highlights;

    static QJsonObject initialize(const QString &rootPath);
    static QJsonObject didOpen(const QString &filePath);
    static QJsonObject didChange(const QString &filePath);
    static QJsonObject didClose(const QString &filePath);
    static QJsonObject hover(const QString &filePath, const Protocol::Position &pos);
    static QJsonObject symbol(const QString &filePath);
    static QJsonObject completion(const QString &filePath, const Protocol::Position &pos);
    static QJsonObject definition(const QString &filePath, const Protocol::Position &pos);
    static QJsonObject signatureHelp(const QString &filePath, const Protocol::Position &pos);
    static QJsonObject references(const QString &filePath, const Protocol::Position &pos);
    static QJsonObject documentHighlight(const QString &filePath, const Protocol::Position &pos);
    static QJsonObject shutdown();
    static QJsonObject exit();
    static QString setHeader(const QJsonObject &object, int requestIndex);
    static QString setHeader(const QJsonObject &object);
    static bool isRequestResult(const QJsonObject &object);
    static bool isRequestError(const QJsonObject &object);
};

class ClientPrivate;
class Client : public QProcess
{
    Q_OBJECT
    ClientPrivate *const d;
public:
    explicit Client(QObject *parent = nullptr);
    static bool exists(const QString &progrma);
    void initRequest(const QString &rootPath); // yes
    void openRequest(const QString &filePath); // no
    void closeRequest(const QString &filePath); // no
    void changeRequest(const QString &filePath); // no
    void symbolRequest(const QString &filePath); // yes
    void hoverRequest(const QString &filePath, const Protocol::Position &pos); // yes
    void definitionRequest(const QString &filePath, const Protocol::Position &pos); // yes
    void completionRequest(const QString &filePath, const Protocol::Position &pos); // yes
    void signatureHelpRequest(const QString &filePath, const Protocol::Position &pos); // yes
    void referencesRequest(const QString &filePath, const Protocol::Position &pos);
    void docHighlightRequest(const QString &filePath, const Protocol::Position &pos);
    void shutdownRequest();
    void exitRequest();

signals:
    void request();
    void notification(const QString &jsonStr);
    void notification(const Protocol::Diagnostics &diagnostics);
    void requestResult(const Protocol::Symbols &symbols);
    void requestResult(const Protocol::Locations &locations);
    void requestResult(const Protocol::CompletionItems &completionItems);
    void requestResult(const Protocol::SignatureHelps &signatureHelps);
    void requestResult(const Protocol::Hover &hover);
    void requestResult(const Protocol::Highlights &highlights);

private:
    bool calledError(const QJsonObject &jsonObj);
    bool calledResult(const QJsonObject &jsonObj); //found result key from json && not found method
    bool initResult(const QJsonObject &jsonObj); // client call server rpc return
    bool openResult(const QJsonObject &jsonObj); // client call server rpc return
    bool symbolResult(const QJsonObject &jsonObj); // client call server rpc return
    bool definitionResult(const QJsonObject &jsonObj); // client call server rpc return
    bool completionResult(const QJsonObject &jsonObj); // client call server rpc return
    bool signatureHelpResult(const QJsonObject &jsonObj); // client call server rpc return
    bool hoverResult(const QJsonObject &jsonObj); // client call server rpc return
    bool referencesResult(const QJsonObject &jsonObj); // client call server rpc return
    bool docHighlightResult(const QJsonObject &jsonObj); // client call server rpc return
    bool closeResult(const QJsonObject &jsonObj); // client call server rpc return
    bool shutdownResult(const QJsonObject &jsonObj);
    bool exitResult(const QJsonObject &jsonObj);

    bool serverCalled(const QJsonObject &jsonObj); // not found result key from json && found key method
    bool diagnostics(const QJsonObject &jsonObj); // server call client

private slots:
    void readJson();
};

} // namespace lsp

#endif // LANGUAGESERVERPROTOCOL_H
