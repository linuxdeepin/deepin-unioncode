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

QJsonObject initialize(const QString &rootPath)
{
    QJsonObject workspace {
        {"workspaceFolders",QJsonObject{ {"supported", true}, {"changeNotifications", true}}}
    };

    QJsonObject semanticTokens{{
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
                {"semanticTokens", semanticTokens}
            }
        },{
            "workspace", workspace
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

    //    QJsonObject workspace {
    //        { "name", QFileInfo(rootPath).fileName() },
    //        { K_URI, QUrl::fromLocalFile(rootPath).toString() }
    //    };

    QJsonArray workspaceFolders{workspace};

    QJsonObject params {
        { K_PROCESSID, QCoreApplication::applicationPid() },
        { K_ROOTPATH, rootPath },
        { K_ROOTURI, rootPath },
        { K_CAPABILITIES, capabilities },
        { K_INITIALIZATIONOPTIONS, V_INITIALIZATIONOPTIONS },
        { "highlight", highlight },
        { "client", client}
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

class ClientPrivate
{
    friend class Client;
    int requestIndex = 0;
    QHash<int, QString> requestSave;
    int semanticTokenResultId = 0;
    QHash<QString, int> fileVersion;
};

Client::Client(QObject *parent)
    : QProcess (parent)
    , d (new ClientPrivate)
{
    QObject::connect(this, &QProcess::readyRead,
                     this, &Client::readJson);
}

Client::~Client()
{
    if (d) {
        delete d;
    }
}

bool Client::exists(const QString &progrma)
{
    return ProcessUtil::exists(progrma);
}

void Client::initRequest(const QString &rootPath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_INITIALIZE);
    qInfo() << "--> server : " << V_INITIALIZE;
    qInfo() << qPrintable(setHeader(initialize(rootPath), d->requestIndex).toLatin1());
    write(setHeader(initialize(rootPath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::openRequest(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DIDOPEN);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDOPEN;
    qInfo() << qPrintable(setHeader(didOpen(filePath), d->requestIndex).toLatin1());
    write(setHeader(didOpen(filePath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::closeRequest(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DIDCLOSE);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDCLOSE
            << qPrintable(setHeader(didClose(filePath), d->requestIndex).toLatin1());
    write(setHeader(didClose(filePath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::changeRequest(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DIDCHANGE);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDCHANGE
            << qPrintable(setHeader(didChange(filePath, d->fileVersion[filePath])));
    write(setHeader(didChange(filePath, d->fileVersion[filePath])).toLatin1());
    waitForBytesWritten();
}

void Client::symbolRequest(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DOCUMENTSYMBOL);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DOCUMENTSYMBOL;
    write(setHeader(symbol(filePath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::definitionRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DEFINITION);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DEFINITION;
    write(setHeader(definition(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::completionRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_COMPLETION);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_COMPLETION
            << qPrintable(setHeader(completion(filePath, pos), d->requestIndex).toLatin1());
    write(setHeader(completion(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::signatureHelpRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_SIGNATUREHELP);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_SIGNATUREHELP;
    write(setHeader(signatureHelp(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::referencesRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_REFERENCES);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_REFERENCES;
    write(setHeader(references(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::docHighlightRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT;
    qInfo() << qPrintable(setHeader(documentHighlight(filePath, pos), d->requestIndex).toLatin1());
    write(setHeader(documentHighlight(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::docSemanticTokensFull(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_SEMANTICTOKENS + "/full");
    qInfo() << "--> server : " << V_TEXTDOCUMENT_SEMANTICTOKENS + "/full";
    qInfo() << qPrintable(setHeader(documentSemanticTokensFull(filePath), d->requestIndex).toLatin1());
    write(setHeader(documentSemanticTokensFull(filePath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::docHoverRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_HOVER);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_HOVER;
    qInfo() << qPrintable(setHeader(hover(filePath, pos), d->requestIndex).toLatin1());
    write(setHeader(hover(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::shutdownRequest()
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_SHUTDOWN);
    qInfo() << "--> server : " << V_SHUTDOWN
            << qPrintable(setHeader(shutdown(), d->requestIndex).toLatin1());
    write(setHeader(shutdown(), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::exitRequest()
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_EXIT);
    qInfo() << "--> server : " << V_EXIT
            << qPrintable(setHeader(exit(), d->requestIndex).toLatin1());
    write(setHeader(exit(), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

bool Client::calledError(const QJsonObject &jsonObj)
{
    if (jsonObj.keys().contains(K_ERROR)) {
        QString errStr = "Failed, called error. code ";
        auto errorObj = jsonObj.value(K_ERROR).toObject();
        auto calledID = jsonObj.value(K_ID).toInt();
        errStr += QString("%0 ").arg(errorObj.value(K_CODE).toInt());
        errStr += QString(",%0 ").arg(errorObj.value(K_MESSAGE).toString());
        if (d->requestSave.keys().contains(calledID)) {
            auto requestMethod = d->requestSave.value(calledID);
            errStr += QString("from: %0").arg(requestMethod);
        }
        d->requestSave.remove(calledID);
        qInfo() << errStr;
        return true;
    }
    return false;
}

bool Client::initResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.values().contains(V_INITIALIZE)
            && d->requestSave.key(V_INITIALIZE) == calledID) {
        qInfo() << "client <-- : " << V_INITIALIZE;
        qInfo() << jsonObj;
        d->requestSave.remove(calledID);

        QJsonObject semanticTokensProviderObj = jsonObj.value("result").toObject()
                .value("capabilities").toObject()
                .value("semanticTokensProvider").toObject();
        QJsonObject fullObj = semanticTokensProviderObj.value("full").toObject();
        QJsonObject legendObj = semanticTokensProviderObj.value("legend").toObject();

        SemanticTokensProvider provider
        {
            SemanticTokensProvider::Full
            {
                fullObj.value("delta").toBool()
            },
            SemanticTokensProvider::Legend
            {
                cvtStringList(legendObj.value("tokenTypes").toArray()),
                        cvtStringList(legendObj.value("tokenModifiers").toArray())
            },
            semanticTokensProviderObj.value("range").toBool()
        };

        emit requestResult(provider);
        return true;
    }
    return false;
}

bool Client::openResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.values().contains(V_TEXTDOCUMENT_DIDOPEN)
            && d->requestSave.key(V_TEXTDOCUMENT_DIDOPEN) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DIDOPEN;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::changeResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.values().contains(V_TEXTDOCUMENT_DIDCHANGE)
            && d->requestSave.key(V_TEXTDOCUMENT_DIDCHANGE) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DOCUMENTSYMBOL;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::symbolResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.values().contains(V_TEXTDOCUMENT_DOCUMENTSYMBOL)
            && d->requestSave.key(V_TEXTDOCUMENT_DOCUMENTSYMBOL) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DOCUMENTSYMBOL;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::definitionResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.values().contains(V_TEXTDOCUMENT_DEFINITION)
            && d->requestSave.key(V_TEXTDOCUMENT_DEFINITION) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DEFINITION;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::completionResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_TEXTDOCUMENT_COMPLETION)
            && d->requestSave.key(V_TEXTDOCUMENT_COMPLETION) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_COMPLETION /*<< jsonObj*/;
        d->requestSave.remove(calledID);
        QJsonObject resultObj = jsonObj.value("result").toObject();
        QJsonArray itemsArray = resultObj.value("items").toArray();
        CompletionProvider completionProvider;
        CompletionItems items;
        for (auto item : itemsArray) {
            QJsonObject itemObj = item.toObject();
            QJsonArray editsArray = itemObj.value("additionalTextEdits").toArray();
            AdditionalTextEdits additionalTextEdits;
            for (auto edit : editsArray) {
                QJsonObject textEditObj = edit.toObject();
                QString newText = textEditObj.value("newText").toString();
                QJsonObject rangeObj = textEditObj.value("range").toObject();
                QJsonObject startObj = rangeObj.value(K_START).toObject();
                QJsonObject endObj = rangeObj.value(K_END).toObject();
                Position start{startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt()};
                Position end{endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt()};
                additionalTextEdits << TextEdit{ newText, Range{ start, end} };
            }

            QJsonObject documentationObj = itemObj.value("documentation").toObject();
            struct Documentation documentation {
                documentationObj.value("kind").toString(), documentationObj.value("value").toString()
            };

            QJsonObject textEditObj = itemObj.value("textEdit").toObject();
            QJsonObject textEditRangeObj = textEditObj.value("range").toObject();
            QJsonObject textEditStartObj = textEditRangeObj.value(K_START).toObject();
            QJsonObject textEditEndObj = textEditRangeObj.value(K_END).toObject();
            QString newText = textEditObj.value("newText").toString();
            Position start{textEditStartObj.value(K_LINE).toInt(), textEditStartObj.value(K_CHARACTER).toInt()};
            Position end{textEditEndObj.value(K_LINE).toInt(), textEditEndObj.value(K_CHARACTER).toInt()};
            TextEdit textEdit{newText, Range{start, end} };

            ;

            items << CompletionItem {
                     additionalTextEdits,
                     documentation,
                     itemObj.value("filterText").toString(),
                     itemObj.value("insertText").toString(),
                     (InsertTextFormat)itemObj.value("insertTextFormat").toInt(),
                     (CompletionItem::Kind)(itemObj.value("kind").toInt()),
                     itemObj.value("label").toString(),
                     itemObj.value("score").toDouble(),
                     itemObj.value("sortText").toString(),
                     textEdit };
        }

        completionProvider.items = items;
        completionProvider.isIncomplete = resultObj.value("isIncomplete").toBool();

        emit requestResult(completionProvider);
        return true;
    }
    return false;
}

bool Client::signatureHelpResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_TEXTDOCUMENT_SIGNATUREHELP)
            && d->requestSave.key(V_TEXTDOCUMENT_SIGNATUREHELP) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_SIGNATUREHELP;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::hoverResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_TEXTDOCUMENT_HOVER)
            && d->requestSave.key(V_TEXTDOCUMENT_HOVER) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_HOVER << jsonObj;
        d->requestSave.remove(calledID);
        QJsonObject resultObj = jsonObj.value("result").toObject();
        QJsonObject contentsObj = resultObj.value("contents").toObject();
        QJsonObject rangeObj = resultObj.value("range").toObject();
        QJsonObject startObj = rangeObj.value(K_START).toObject();
        QJsonObject endObj = rangeObj.value(K_END).toObject();
        Hover hover {
            Contents { contentsObj.value("kind").toString(), contentsObj.value("value").toString() },
            Range {
                Position { startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt() },
                Position { endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt() }
            }
        };

        emit requestResult(hover);
        return true;
    }
    return false;
}

bool Client::referencesResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_TEXTDOCUMENT_REFERENCES)
            && d->requestSave.key(V_TEXTDOCUMENT_REFERENCES) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_REFERENCES;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::docHighlightResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT)
            && d->requestSave.key(V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT
                << jsonObj;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::docSemanticTokensFullResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_TEXTDOCUMENT_SEMANTICTOKENS + "/full")
            && d->requestSave.key(V_TEXTDOCUMENT_SEMANTICTOKENS + "/full") == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_SEMANTICTOKENS + "full"
                << jsonObj;
        d->requestSave.remove(calledID);

        QJsonObject result = jsonObj.value(K_RESULT).toObject();
        d->semanticTokenResultId = jsonObj.value("resultId").toInt();

        QJsonArray dataArray = result.value(K_DATA).toArray();
        if(dataArray.isEmpty())
            return true;

        QList<Data> results;
        auto itera = dataArray.begin();
        while (itera != dataArray.end()) {
            results << Data {
                       Position{itera++->toInt(), itera++->toInt()},
                       int(itera++->toInt()),
                       (SemanticTokenType)itera++->toInt(),
                       fromTokenModifiers(itera++->toInt())};
        }
        emit requestResult(results);
        return true;
    }
    return false;
}

bool Client::closeResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_TEXTDOCUMENT_DIDCLOSE)
            && d->requestSave.key(V_TEXTDOCUMENT_DIDCLOSE) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DIDCLOSE;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::exitResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_EXIT)
            && d->requestSave.key(V_EXIT) == calledID) {
        qInfo() << "client <-- : " << V_EXIT
                << jsonObj;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::shutdownResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_SHUTDOWN)
            && d->requestSave.key(V_SHUTDOWN) == calledID) {
        qInfo() << "client <-- : " << V_SHUTDOWN
                << jsonObj;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::diagnostics(const QJsonObject &jsonObj)
{
    Diagnostics diagnostics;
    if (!jsonObj.keys().contains(K_METHOD)
            || jsonObj.value(K_METHOD).toString() != V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS)
        return false;
    qInfo() << "client <-- : " << V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS;

    QJsonArray array = jsonObj.value(K_PARAMS).toObject().value(K_DIAGNOSTICS).toArray();

    for (auto val : array) {
        QJsonObject diagnosticObj = val.toObject();
        QJsonObject rangeObj = diagnosticObj.value(K_RANGE).toObject();
        QJsonObject startObj = rangeObj.value(K_START).toObject();
        QJsonObject endObj = rangeObj.value(K_END).toObject();
        Diagnostic diagnostic
        {
            QString { diagnosticObj.value(K_MESSAGE).toString() },
            Range {
                Position { startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt()},
                Position { endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt()}
            },
            diagnosticObj.value(K_SEVERITY).toInt()
        };
        diagnostics << diagnostic;
    }

    emit notification(diagnostics);
    return true;
}

bool Client::serverCalled(const QJsonObject &jsonObj)
{
    if (diagnostics(jsonObj))
        return true;

    return false;
}

bool Client::calledResult(const QJsonObject &jsonObj)
{
    if (initResult(jsonObj))
        return true;

    if (openResult(jsonObj))
        return true;

    if (symbolResult(jsonObj))
        return true;

    if (definitionResult(jsonObj))
        return true;

    if (completionResult(jsonObj))
        return true;

    if (signatureHelpResult(jsonObj))
        return true;

    if (hoverResult(jsonObj))
        return true;

    if (docHighlightResult(jsonObj))
        return true;

    if (docSemanticTokensFullResult(jsonObj))
        return true;

    if (closeResult(jsonObj))
        return true;

    if (shutdownResult(jsonObj))
        return true;

    if (exitResult(jsonObj))
        return true;

    return false;
}

void Client::readJson()
{
    static QString readed;
    static int readedCount = 0;
    QString current;
    int currentCount = 0;
    QList<QJsonObject> jsonObjs;
    qInfo() << "bytesAvailable: " << bytesAvailable();
    while (bytesAvailable()) {
        current += read(1);
        if (!readed.isEmpty() && readedCount != 0) {
            if (readed.length() + current.length() == readedCount) {
                processJson(QJsonDocument::fromJson((readed += current).toLatin1()).object());
                current.clear();
                readed.clear();
                readedCount = 0;
                qInfo() << "call next end";
                continue;
            } else if (readed.length() + current.length() < readedCount) {
                readed += current;
                current.clear();
            } else {
                qCritical() << "error";
            }
        }

        if (current.contains("\r\n\r\n")) {
            auto list = current.split("\r\n\r\n");
            auto contentLength = list[0].split(":");
            if (contentLength.size() != 2) {
                qCritical() << "Failed, Process Header error size != 2";
                return;
            }
            if (contentLength.at(0) != H_CONTENT_LENGTH){
                qCritical() << "Failed, Process Header error, not found H_CONTENT_LENGTH";
                return;
            }
            currentCount = contentLength[1].toInt();
            current.clear();
            current += list[1];
            break;
        }
    }

    if (currentCount == 0) {
        qInfo() << "Failed, can process jsonLenght is 0";
    }

    current += read(currentCount);
    //to UTF8 code size
    qInfo() << current.toUtf8().length()
            << currentCount;
    if (current.toUtf8().length() < currentCount) {
        qInfo() << "read json lenght not real lenght, data for next";
        readedCount = currentCount;
        readed = current;
        readJson();
    }

    processJson(QJsonDocument::fromJson(current.toLatin1()).object());
}

void Client::processJson(const QJsonObject &jsonObj)
{
    if (calledError(jsonObj))
        return;

    if (calledResult(jsonObj))
        return;

    if (serverCalled(jsonObj))
        return;
}

QStringList Client::cvtStringList(const QJsonArray &array)
{
    QStringList ret;
    for (auto val : array) {
        ret << val.toString();
    }
    return ret;
}

}
