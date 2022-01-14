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

enum SemanticTokenType {
    Namespace = 0,
    Type,
    Class,
    Enum,
    Interface,
    Struct,
    TypeParameter,
    Parameter,
    Variable,
    Property,
    EnumMember,
    Event,
    Function,
    Method,
    Macro,
    Keyword,
    Modifier,
    Comment,
    String,
    Number,
    Regexp,
    Operator
};

enum SemanticTokenModifier {
    Declaration = 0, //声明
    Definition,  //定义
    Readonly,
    Static,
    Deprecated,
    Abstract,
    Async,
    Modification,
    Documentation,
    DefaultLibrary
};

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

enum InsertTextFormat
{
    PlainText = 1,
    Snippet = 2,
};

struct TextEdit
{
    QString newText;
    Range range;
};

typedef QList<TextEdit> AdditionalTextEdits;

struct Documentation
{
    QString kind; // markdown or plaintext
    QString value;
};

struct CompletionItem
{
    enum Kind {
        Text = 1,
        Method = 2,
        Function = 3,
        Constructor = 4,
        Field = 5,
        Variable = 6,
        Class = 7,
        Interface = 8,
        Module = 9,
        Property = 10,
        Unit = 11,
        Value = 12,
        Enum = 13,
        Keyword = 14,
        Snippet = 15,
        Color = 16,
        File = 17,
        Reference = 18,
        Folder = 19,
        EnumMember = 20,
        Constant = 21,
        Struct = 22,
        Event = 23,
        Operator = 24,
        TypeParameter = 25
    };
    AdditionalTextEdits additionalTextEdits;
    struct Documentation documentation;
    QString filterText;
    QString insertText;
    InsertTextFormat insertTextFormat;
    CompletionItem::Kind kind;
    QString label;
    double score;
    QString sortText;
    TextEdit textEdit;
};

typedef QList<CompletionItem> CompletionItems;

struct CompletionProvider
{
    bool isIncomplete;
    CompletionItems items;
};

struct SignatureHelp //暂时留空
{

};
typedef QList<SignatureHelp> SignatureHelps;

struct Contents
{
    QString kind;
    QString value;
};

struct Hover //暂时留空
{
    Contents contents;
    Range range;
};

struct Highlight //暂时留空
{

};
typedef QList<Highlight> Highlights;

struct Data //from result key "data"
{
    Position start;
    int length;
    SemanticTokenType tokenType;
    QList<SemanticTokenModifier> tokenModifiers;
};

struct SemanticTokensProvider
{
    struct Full{bool delta;};
    struct Legend{
        QStringList tokenTypes;
        QStringList tokenModifiers;
    };
    Full full;
    Legend legend;
    bool range;
};

enum TextDocumentSyncKind
{
    None = 0,
    Full = 1,
    Incremental = 2
};

struct TextDocumentSyncOptions
{
    bool openColse;
    TextDocumentSyncKind change;
};

struct TextDocumentIdentifier
{
    QUrl documentUri;
};

struct VersionedTextDocumentIdentifier: public TextDocumentIdentifier
{
    int version;
};

struct TextDocumentPositionParams
{
    TextDocumentIdentifier textDocument;
    Position position;
};

struct TextDocumentItem
{
    QUrl DocumentUri;
    QString languageId;
    int version;
    QString text;
};

struct TextDocumentContentChangeEvent
{
    Range range;
    int rangeLength;
    QString text;
};

struct DidChangeTextDocumentParams
{
    VersionedTextDocumentIdentifier textDocument;
    QList<TextDocumentContentChangeEvent> contentChanges;
};

QString fromTokenType(SemanticTokenType type);
QString fromTokenModifier(SemanticTokenModifier modifier);
QList<SemanticTokenModifier> fromTokenModifiers(int modifiers);
QJsonArray tokenTypes();
QJsonArray tokenModifiers();

QJsonObject workspace();
QJsonObject initialize(const QString &rootPath);
QJsonObject didOpen(const QString &filePath);
QJsonObject didChange(const QString &filePath, int version);
QJsonObject didClose(const QString &filePath);
QJsonObject hover(const QString &filePath, const Position &pos);
QJsonObject symbol(const QString &filePath);
QJsonObject completion(const QString &filePath, const Position &pos);
QJsonObject definition(const QString &filePath, const Position &pos);
QJsonObject signatureHelp(const QString &filePath, const Position &pos);
QJsonObject references(const QString &filePath, const Position &pos);
QJsonObject documentHighlight(const QString &filePath, const Position &pos);
QJsonObject documentSemanticTokensFull(const QString &filePath);
QJsonObject documentSemanticTokensRange(const QString &filePath);
QJsonObject documentSemanticTokensDelta(const QString &filePath);
QJsonObject shutdown();
QJsonObject exit();
QString setHeader(const QJsonObject &object, int requestIndex);
QString setHeader(const QJsonObject &object);
bool isRequestResult(const QJsonObject &object);
bool isRequestError(const QJsonObject &object);

class ClientPrivate;
class Client : public QProcess
{
    Q_OBJECT
    ClientPrivate *const d;
public:
    explicit Client(QObject *parent = nullptr);
    virtual ~Client();
    static bool exists(const QString &progrma);
    void initRequest(const QString &rootPath); // yes
    void openRequest(const QString &filePath); // no
    void closeRequest(const QString &filePath); // no
    void changeRequest(const QString &filePath); // no
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

signals:
    void request();
    void notification(const QString &jsonStr);
    void notification(const Diagnostics &diagnostics);
    void requestResult(const SemanticTokensProvider &tokensProvider);
    void requestResult(const Symbols &symbols);
    void requestResult(const Locations &locations);
    void requestResult(const CompletionProvider &completionProvider);
    void requestResult(const SignatureHelps &signatureHelps);
    void requestResult(const Hover &hover);
    void requestResult(const Highlights &highlights);
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
    void processJson(const QJsonObject &jsonObj);

private slots:
    void readJson();
    QStringList cvtStringList(const QJsonArray &array);
};

} // namespace lsp

#endif // LANGUAGESERVERPROTOCOL_H
