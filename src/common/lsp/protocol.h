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

extern const QString K_ID;
extern const QString K_JSON_RPC;
extern const QString K_METHOD;
extern const QString K_RESULT;
extern const QString K_PARAMS;
extern const QString K_CAPABILITIES;
extern const QString K_TEXTDOCUMENT;
extern const QString K_DOCUMENTSYMBOL;
extern const QString K_HIERARCHICALDOCUMENTSYMBOLSUPPORT;
extern const QString K_PUBLISHDIAGNOSTICS;
extern const QString K_RELATEDINFOMATION;
extern const QString K_INITIALIZATIONOPTIONS;
extern const QString K_PROCESSID;
extern const QString K_ROOTPATH;
extern const QString K_ROOTURI;
extern const QString K_URI; // value QString from file url
extern const QString K_VERSION; // value int
extern const QString K_LANGUAGEID;
extern const QString K_TEXT;
extern const QString K_CONTAINERNAME;
extern const QString K_KIND;
extern const QString K_LOCATION;
extern const QString K_POSITION;
extern const QString K_DATA;

extern const QString H_CONTENT_LENGTH;
extern const QString V_2_0;
extern const QString V_INITIALIZE; //has request result
extern const QString V_SHUTDOWN; //has request result
extern const QString V_EXIT; //has request result
extern const QString V_TEXTDOCUMENT_DIDOPEN; //no request result
extern const QString V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS; //server call
extern const QString V_TEXTDOCUMENT_DIDCHANGE; //no request result, json error
extern const QString V_TEXTDOCUMENT_DOCUMENTSYMBOL; // has request result
extern const QString V_TEXTDOCUMENT_HOVER; // has request result
extern const QString V_TEXTDOCUMENT_DEFINITION ;
extern const QString V_TEXTDOCUMENT_DIDCLOSE;
extern const QString V_TEXTDOCUMENT_COMPLETION;
extern const QString V_TEXTDOCUMENT_SIGNATUREHELP;
extern const QString V_TEXTDOCUMENT_REFERENCES;
extern const QString V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT;
extern const QString V_TEXTDOCUMENT_SEMANTICTOKENS;
extern const QString K_WORKSPACEFOLDERS;

extern const QString K_CONTENTCHANGES;
extern const QString K_DIAGNOSTICS;
extern const QString K_RANGE;
extern const QString K_MESSAGE;
extern const QString K_SEVERITY;
extern const QString K_END;
extern const QString K_START;
extern const QString K_CHARACTER;
extern const QString K_LINE;
extern const QString K_CONTEXT;
extern const QString K_INCLUDEDECLARATION;

extern const QString K_ERROR;
extern const QString K_CODE;

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


} // namespace lsp

#endif // LANGUAGESERVERPROTOCOL_H
