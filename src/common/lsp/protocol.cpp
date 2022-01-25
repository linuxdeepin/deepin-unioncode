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
#include "protocol.h"
#include "common/util/processutil.h"

#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QQueue>
#include <QUrl>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

namespace lsp {

const QString K_ID {"id"};
const QString K_JSON_RPC {"jsonrpc"};
const QString K_METHOD {"method"};
const QString K_RESULT {"result"};
const QString K_PARAMS {"params"};
const QString K_CAPABILITIES {"capabilities"};
const QString K_TEXTDOCUMENT {"textDocument"};
const QString K_DOCUMENTSYMBOL {"documentSymbol"};
const QString K_HIERARCHICALDOCUMENTSYMBOLSUPPORT {"hierarchicalDocumentSymbolSupport"};
const QString K_PUBLISHDIAGNOSTICS {"publishDiagnostics"};
const QString K_RELATEDINFOMATION {"relatedInformation"};
const QString K_INITIALIZATIONOPTIONS {"initializationOptions"};
const QString K_PROCESSID {"processId"};
const QString K_ROOTPATH {"rootPath"};
const QString K_ROOTURI {"rootUri"};
const QString K_URI {"uri"}; // value QString from file url
const QString K_VERSION {"version"}; // value int
const QString K_LANGUAGEID {"languageId"};
const QString K_TEXT {"text"};
const QString K_CONTAINERNAME {"containerName"};
const QString K_KIND {"kind"};
const QString K_LOCATION {"location"};
const QString K_POSITION {"position"};
const QString K_DATA{"data"};

const QString H_CONTENT_LENGTH {"Content-Length"};
const QString V_2_0 {"2.0"};
const QString V_INITIALIZE {"initialize"}; //has request result
const QString V_SHUTDOWN {"shutdown"}; //has request result
const QString V_EXIT{"exit"}; //has request result
const QString V_TEXTDOCUMENT_DIDOPEN {"textDocument/didOpen"}; //no request result
const QString V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS {"textDocument/publishDiagnostics"}; //server call
const QString V_TEXTDOCUMENT_DIDCHANGE {"textDocument/didChange"}; //no request result, json error
const QString V_TEXTDOCUMENT_DOCUMENTSYMBOL {"textDocument/documentSymbol"}; // has request result
const QString V_TEXTDOCUMENT_HOVER {"textDocument/hover"}; // has request result
const QString V_TEXTDOCUMENT_DEFINITION {"textDocument/definition"};
const QString V_TEXTDOCUMENT_DIDCLOSE {"textDocument/didClose"};
const QString V_TEXTDOCUMENT_COMPLETION {"textDocument/completion"};
const QString V_TEXTDOCUMENT_SIGNATUREHELP {"textDocument/signatureHelp"};
const QString V_TEXTDOCUMENT_REFERENCES {"textDocument/references"};
const QString V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT {"textDocument/documentHighlight"};
const QString V_TEXTDOCUMENT_SEMANTICTOKENS {"textDocument/semanticTokens"};
const QString K_WORKSPACEFOLDERS {"workspaceFolders"};

const QString K_CONTENTCHANGES {"contentChanges"};
const QString K_DIAGNOSTICS {"diagnostics"};
const QString K_RANGE {"range"};
const QString K_MESSAGE {"message"};
const QString K_SEVERITY {"severity"};
const QString K_END {"end"};
const QString K_START {"start"};
const QString K_CHARACTER {"character"};
const QString K_LINE {"line"};
const QString K_CONTEXT {"context"};
const QString K_INCLUDEDECLARATION {"includeDeclaration"};

const QString K_ERROR {"error"};
const QString K_CODE {"code"};

const QJsonValue V_INITIALIZATIONOPTIONS(QJsonValue::Null);

QString fromTokenType(SemanticTokenType type)
{
    switch (type) {
    case Namespace:
        return "namespace";
    case Type:
        return "type";
    case Class:
        return "class";
    case Enum:
        return "enum";
    case Interface:
        return "interface";
    case Struct:
        return "struct";
    case TypeParameter:
        return "typeParameter";
    case Parameter:
        return "parameter";
    case Variable:
        return "variable";
    case Property:
        return "property";
    case EnumMember:
        return "enumMember";
    case Event:
        return "event";
    case Function:
        return "function";
    case Method:
        return "method";
    case Macro:
        return "macro";
    case Keyword:
        return "keyword";
    case Modifier:
        return "modifier";
    case Comment:
        return "comment";
    case String:
        return "string";
    case Number:
        return "number";
    case Regexp:
        return "regexp";
    case Operator:
        return "operator";
    }

    return "";
}

QString fromTokenModifier(SemanticTokenModifier modifier)
{
    switch (modifier) {
    case Declaration:
        return "declaration";
    case Definition:
        return "definition";
    case Readonly:
        return "readonly";
    case Static:
        return "static";
    case Deprecated:
        return "deprecated";
    case Abstract:
        return "abstract";
    case Async:
        return "async";
    case Modification:
        return "modification";
    case Documentation:
        return "documentation";
    case DefaultLibrary:
        return "defaultLibrary";
    }
    return "";
}

QList<SemanticTokenModifier> fromTokenModifiers(int modifiers)
{
    QList<SemanticTokenModifier> ret;
    int temp = modifiers;
    ret.push_front((SemanticTokenModifier)(temp %10));
    while (temp / 10 >= 1) {
        temp = temp / 10;
        ret.push_front((SemanticTokenModifier)(temp %10));
    }

    return ret;
}

QJsonArray tokenTypes()
{
    return {
        "namespace",
        "type",
        "class",
        "enum",
        "interface",
        "struct",
        "typeParameter",
        "parameter",
        "variable",
        "property",
        "enumMember",
        "event",
        "function",
        "method",
        "macro",
        "keyword",
        "modifier",
        "comment",
        "string",
        "number",
        "regexp",
        "operator"
    };
}

QJsonArray tokenModifiers()
{
    return {
        "declaration",
        "definition",
        "readonly",
        "static",
        "deprecated",
        "abstract",
        "async",
        "modification",
        "documentation",
        "defaultLibrary"
    };
}

QJsonObject workspace()
{
    QJsonObject didChangeConfiguration {
        { "dynamicRegistration", true }
    };
    QJsonObject codeLens {
        { "refreshSupport", true }
    };
    QJsonObject didChangeWatchedFiles {
        { "dynamicRegistration", true}
    };
    QJsonObject executeCommand{
        { "dynamicRegistration", true}
    };
    QJsonObject fileOperations {
        { "didCreate", true},
        { "didDelete", true},
        { "didRename", true},
        { "dynamicRegistration", true},
        { "willCreate", true},
        { "willDelete", true},
        { "willRename", true}
    };
    QJsonObject semanticTokens{
        { "refreshSupport", true }
    };

    QJsonArray symbolKind_valueSet { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26 };
    QJsonObject symbolKind{ { "valueSet", symbolKind_valueSet } };
    QJsonArray tagSupport_valueSet{1};
    QJsonObject tagSupport{ { "valueSet", tagSupport_valueSet } };
    QJsonObject symbol{
        { "dynamicRegistration", true},
        { "symbolKind", symbolKind },
        { "tagSupport", tagSupport }
    };

    QJsonArray resourceOperations{
        { "create", "rename", "delete" }
    };

    QJsonObject changeAnnotationSupport{
        { "groupsOnLabel", true }
    };
    QJsonObject workspaceEdit{
        { "changeAnnotationSupport",changeAnnotationSupport },
        { "documentChanges", true },
        { "failureHandling", "textOnlyTransactional" },
        { "normalizesLineEndings", true },
        { "resourceOperations", resourceOperations}
    };

    QJsonObject workspace {
        { "applyEdit", true },
        { "codeLens", codeLens },
        { "configuration", true },
        { "didChangeConfiguration", didChangeConfiguration },
        { "didChangeWatchedFiles", didChangeWatchedFiles },
        { "executeCommand", executeCommand },
        { "fileOperations", fileOperations },
        { "semanticTokens", semanticTokens },
        { "symbol",symbol },
        { "workspaceEdit", workspaceEdit },
        { "workspaceFolders", true }
    };

    return workspace;
}

QJsonObject initialize(const QString &rootPath)
{
    QJsonObject capabilitiesSemanticTokens{{
            {"dynamicRegistration", true},
            {"formats", QJsonArray{"relative"}},
            {"multilineTokenSupport", false},
            {"overlappingTokenSupport", false},
            {"requests", QJsonArray{"full",QJsonObject{{"delta", true}},QJsonObject{{"range", true}}}},
            {"tokenModifiers", tokenModifiers()},
            {"tokenTypes", tokenTypes()}}};

    QJsonObject capabilities {
        {
            K_TEXTDOCUMENT, QJsonObject {
                {"documentLink", QJsonObject{{"dynamicRegistration", true}}},
                {"documentHighlight", QJsonObject{{"dynamicRegistration", true}}},
                {K_DOCUMENTSYMBOL, QJsonObject{{K_HIERARCHICALDOCUMENTSYMBOLSUPPORT, true}},},
                {K_PUBLISHDIAGNOSTICS, QJsonObject{{K_RELATEDINFOMATION, true}}},
                {"definition", QJsonObject{{{"dynamicRegistration", true},{"linkSupport", true}}}},
                {"colorProvider", QJsonObject{{"dynamicRegistration", true}}},
                {"declaration", QJsonObject{{"dynamicRegistration", true},{"linkSupport", true}}},
                {"semanticHighlightingCapabilities", QJsonObject{{"semanticHighlighting", true}}},
                {"semanticTokens", capabilitiesSemanticTokens}
            }
        },{
            "workspace", workspace()
        },{
            "foldingRangeProvider", true,
        }
    };

    QJsonObject highlight {
        {"largeFileSize", 2097152},
        {"lsRanges", false},
        {"blacklist", QJsonArray()},
        {"whitelist", QJsonArray()}
    };

    QJsonObject client{
        {"diagnosticsRelatedInformation", true},
        {"hierarchicalDocumentSymbolSupport", true},
        {"linkSupport",true},
        {"snippetSupport",true},
    };

    QJsonObject workspace {
        { "name", QFileInfo(rootPath).fileName() },
//        { K_URI, QUrl::fromLocalFile(rootPath + QDir::separator() + ".unioncode").toString() }
        { K_URI, QUrl::fromLocalFile(rootPath).toString() }
    };

    QJsonArray workspaceFolders{workspace};

    QJsonObject params {
        { K_PROCESSID, QCoreApplication::applicationPid() },
        { K_ROOTPATH, rootPath },
        { K_ROOTURI, rootPath },
        { K_CAPABILITIES, capabilities },
        { K_INITIALIZATIONOPTIONS, V_INITIALIZATIONOPTIONS },
        { "highlight", highlight },
        { "client", client },
        { "workspaceFolders", workspaceFolders }
    };

    QJsonObject initRequest{
        { K_METHOD, V_INITIALIZE},
        { K_PARAMS, params },
    };

    return initRequest;
}

QJsonObject didOpen(const QString &filePath)
{
    QFile file(filePath);
    QString text;
    if (!file.open(QFile::ReadOnly)) {
        qCritical()<< "Failed, open file: "
                   << filePath <<file.errorString();
    }
    text = file.readAll();
    file.close();

    QJsonObject textDocument{
        { K_URI, QUrl::fromLocalFile(filePath).toString() },
        { K_LANGUAGEID, "cpp" },
        { K_VERSION, 0 },
        { K_TEXT, text }
    };

    QJsonObject params{
        { K_TEXTDOCUMENT, textDocument }
    };

    QJsonObject didOpenRequest {
        { K_METHOD, V_TEXTDOCUMENT_DIDOPEN },
        { K_PARAMS, params}
    };

    return didOpenRequest;
}

// full mode
QJsonObject didChange(const QString &filePath, int version)
{
    QFile file(filePath);
    QString text;
    if (!file.open(QFile::ReadOnly)) {
        qCritical()<< "Failed, open file: "
                   << filePath <<file.errorString();
    }
    text = file.readAll();
    file.close();

    QJsonObject changeEvent
    {
        { K_TEXT, text}
    };

    QJsonArray contentChanges
    {
        changeEvent
    };

    QJsonObject textDocument
    {
        { K_URI, QUrl::fromLocalFile(filePath).toString() },
        { K_VERSION, version }
    };

    QJsonObject params{
        { K_TEXTDOCUMENT, textDocument },
        { "contentChanges", contentChanges }
    };

    QJsonObject didChangeRequest{
        { K_METHOD, V_TEXTDOCUMENT_DIDCHANGE },
        { K_PARAMS, params}
    };

    return didChangeRequest;
}

QJsonObject didClose(const QString &filePath)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject params{
        { K_TEXTDOCUMENT, textDocument},
    };

    QJsonObject didClose {
        { K_METHOD, V_TEXTDOCUMENT_DIDCLOSE },
        { K_PARAMS, params}
    };

    return didClose;
}

QJsonObject hover(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument{
        { K_URI, QUrl::fromLocalFile(filePath).toString() },
    };

    QJsonObject position{
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params{
        { K_TEXTDOCUMENT, textDocument},
        { K_POSITION, position }
    };

    QJsonObject hover {
        { K_METHOD, V_TEXTDOCUMENT_HOVER },
        { K_PARAMS, params }
    };

    return hover;
}

QJsonObject definition(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position}
    };

    QJsonObject definition {
        { K_METHOD, V_TEXTDOCUMENT_DEFINITION },
        { K_PARAMS, params }
    };

    return definition;
}

QJsonObject signatureHelp(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position}
    };

    QJsonObject signatureHelp{
        { K_METHOD, V_TEXTDOCUMENT_SIGNATUREHELP },
        { K_PARAMS, params}
    };

    return signatureHelp;
}

QJsonObject references(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject context {
        { K_INCLUDEDECLARATION, true}
    };

    QJsonObject params {
        { K_CONTEXT, context},
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position}
    };

    QJsonObject signatureHelp{
        { K_METHOD, V_TEXTDOCUMENT_REFERENCES },
        { K_PARAMS, params}
    };

    return signatureHelp;
}

QJsonObject documentHighlight(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position }
    };

    QJsonObject highlight {
        { K_METHOD, V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT},
        { K_PARAMS, params}
    };

    return highlight;
}

//more see https://microsoft.github.io/language-server-protocol/specifications/specification-current/#textDocument_semanticTokens
QJsonObject documentSemanticTokensFull(const QString &filePath)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument }
    };

    QJsonObject semanticTokensFull {
        { K_METHOD, V_TEXTDOCUMENT_SEMANTICTOKENS + "/full"},
        { K_PARAMS, params}
    };

    return semanticTokensFull;
}

QJsonObject documentSemanticTokensDelta(const QString &filePath)
{

}

QJsonObject shutdown()
{
    QJsonObject shutdown {
        { K_METHOD, V_SHUTDOWN }
    };

    return shutdown;
}

QJsonObject exit()
{
    QJsonObject exit {
        { K_METHOD, V_EXIT },
        { K_PARAMS, QJsonValue(QJsonValue::Object)}
    };
    return exit;
}

QJsonObject symbol(const QString &filePath)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument }
    };

    QJsonObject documentSymbol {
        { K_METHOD, V_TEXTDOCUMENT_DOCUMENTSYMBOL},
        { K_PARAMS, params}
    };

    return documentSymbol;
}

QJsonObject completion(const QString &filePath, const Position &pos)
{
    QJsonObject textDocument {
        { K_URI, QUrl::fromLocalFile(filePath).toString() }
    };

    QJsonObject position {
        { K_CHARACTER, pos.character },
        { K_LINE, pos.line }
    };

    QJsonObject params {
        { K_TEXTDOCUMENT, textDocument },
        { K_POSITION, position}
    };

    QJsonObject completion {
        { K_METHOD, V_TEXTDOCUMENT_COMPLETION},
        { K_PARAMS, params}
    };

    return completion;
}

QString setHeader(const QJsonObject &object, int requestIndex)
{
    auto jsonObj = object;
    jsonObj.insert(K_JSON_RPC, V_2_0);
    if (object.value(K_METHOD) != V_TEXTDOCUMENT_DIDOPEN)
        jsonObj.insert(K_ID, requestIndex);
    QJsonDocument jsonDoc(jsonObj);
    QString jsonStr = jsonDoc.toJson();
    return H_CONTENT_LENGTH + QString(": %0\r\n\r\n").arg(jsonStr.length()) + jsonStr;
}

QString setHeader(const QJsonObject &object)
{
    auto jsonObj = object;
    jsonObj.insert(K_JSON_RPC, V_2_0);
    QJsonDocument jsonDoc(jsonObj);
    QString jsonStr = jsonDoc.toJson();
    return H_CONTENT_LENGTH + QString(": %0\r\n\r\n").arg(jsonStr.length()) + jsonStr;
}

bool isRequestResult(const QJsonObject &object)
{
    QStringList keys = object.keys();
    if (keys.contains(K_ID) && keys.contains(K_RESULT))
        return true;
    return false;
}

bool isRequestError(const QJsonObject &object)
{
    if (object.keys().contains(K_ERROR)) {
        qInfo() << "Failed, Request error";
        return true;
    }
    return false;
}

}
