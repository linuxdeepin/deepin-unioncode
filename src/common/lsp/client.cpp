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
#include "client.h"

#include "jsonrpccpp/client.h"
#include "jsonrpccpp/client/connectors/tcpsocketclient.h"

#include "common/util/processutil.h"

#include <QMetaType>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRunnable>
#include <QThreadPool>
#include <QCoreApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QtConcurrent>
#include <QHostAddress>
#include <QReadWriteLock>

namespace lsp {

static QMutex mutex;
class Client;
class ClientPrivate : public jsonrpc::TcpSocketClient
{
    friend class Client;
    Client *const q;
    int requestIndex;
    QHash<int, QString> requestSave;
    int semanticTokenResultId;
    QHash<QString, int> fileVersion;
    lsp::SemanticTokensProvider secTokensProvider;
    lsp::Head head;
    ClientPrivate() = delete;
    ClientPrivate(Client *const q, const std::string &ipToConnect, const unsigned int &port)
        : jsonrpc::TcpSocketClient(ipToConnect, port)
        , q (q)
        , requestIndex (20)
        , requestSave ({})
        , semanticTokenResultId (0)
        , fileVersion ({})
        , secTokensProvider ({})
        , head ({})
    {

    }

    void callLanguageRequest(const QJsonObject &request);
    void callLanguageRequest(const QString &request);
    void callLanguageNotify(const QJsonObject &notify);
    void callLanguageNotify(const QString &notify);
    void responseProcess(const QByteArray &result);
};

Client::Client(unsigned int port, const QString &host)
    : d (new ClientPrivate(this, host.toStdString(), port))
{
    qRegisterMetaType<lsp::Diagnostics>("lsp::Diagnostics");
    qRegisterMetaType<lsp::SemanticTokensProvider>("lsp::SemanticTokensProvider");
    qRegisterMetaType<lsp::Symbols>("lsp::Symbols");
    qRegisterMetaType<lsp::Locations>("lsp::Locations");
    qRegisterMetaType<lsp::CompletionProvider>("lsp::CompletionProvider");
    qRegisterMetaType<lsp::SignatureHelps>("lsp::SignatureHelps");
    qRegisterMetaType<lsp::Highlights>("lsp::Highlights");
    qRegisterMetaType<QList<lsp::Data>>("QList<lsp::Data>");
    qRegisterMetaType<lsp::DefinitionProvider>("lsp::DefinitionProvider");
    qRegisterMetaType<lsp::DiagnosticsParams>("lsp::DiagnosticsParams");
    qRegisterMetaType<lsp::Data>("lsp::Data");
    qRegisterMetaType<lsp::References>("lsp::References");
    qRegisterMetaType<lsp::Position>("lsp::Position");
    qRegisterMetaType<newlsp::Hover>("newlsp::Hover");
    qRegisterMetaType<newlsp::WorkspaceEdit>("newlsp::WorkspaceEdit");
    qRegisterMetaType<newlsp::Position>("newlsp::Position");
    qRegisterMetaType<newlsp::Range>("newlsp::Range");
    qRegisterMetaType<newlsp::PublishDiagnosticsParams>("newlsp::PublishDiagnosticsParams");
}

void Client::setHead(const lsp::Head &head)
{
    d->head = head;
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

void ClientPrivate::callLanguageRequest(const QJsonObject &request)
{
    if (!head.isValid())
        return;

    callLanguageRequest(QJsonDocument(request).toJson());
}

void ClientPrivate::callLanguageRequest(const QString &request)
{
    if (!head.isValid())
        return;

    std::string data = request.toStdString();
    Json::Value orgParams;
    orgParams["workspace"] = head.workspace.toStdString();
    orgParams["language"] = head.language.toStdString();
    orgParams["data"] = data;

    auto resultJsonValue = jsonrpc::Client(*this).CallMethod("fromRequest", orgParams);
    this->responseProcess(QByteArray::fromStdString(resultJsonValue["data"].asString()));
}

void ClientPrivate::callLanguageNotify(const QJsonObject &notify)
{
    if (!head.isValid())
        return;

    callLanguageNotify(QJsonDocument(notify).toJson());
}

void ClientPrivate::callLanguageNotify(const QString &notify)
{
    if (!head.isValid())
        return;

    std::string data = notify.toStdString();
    Json::Value orgParams;
    orgParams["workspace"] = head.workspace.toStdString();
    orgParams["language"] = head.language.toStdString();
    orgParams["data"] = data;

    jsonrpc::Client(*this).CallNotification("fromNotify", orgParams);
}

void ClientPrivate::responseProcess(const QByteArray &result)
{
    QByteArray temp = result;
    QString contentLengthKey{"ContentLength"};
    QString contentTypeKey{"ContentType"};
    QString charsetKey{"charset"};
    QRegularExpression headRegExp("^Content-Length:\\s?(?<" + contentLengthKey + ">[0-9]+)\\r\\n" +
                                  "(Content-Type:\\s?(?<" + contentTypeKey + ">\\S+);" +
                                  "\\s?charset=(?<" + charsetKey + ">\\S+)\\r\\n)?\\r\\n");
    QByteArray head, body;
    for (int idx = 0; idx < temp.size(); idx++) {
        auto matchs = headRegExp.match(head);
        if (!matchs.hasMatch()) {
            head += temp[idx];
        } else {
            int contentLength = matchs.captured(contentLengthKey).toInt();
            QString contentType = matchs.captured(contentTypeKey);
            QString charset = matchs.captured(charsetKey);

            if (contentLength > body.size()) {
                body += result[idx];
            }

            if (contentLength == body.size()) {
                QJsonParseError err;
                QJsonObject jsonObj = QJsonDocument::fromJson(body, &err).object();
                if (err.error != QJsonParseError::NoError) {
                    qInfo() << "covert json error"
                            << err.errorString();
                } else {
                    q->processJson(jsonObj);
                    temp = temp.remove(0, idx + 1);
                    if (!temp.isEmpty())
                        return responseProcess(temp);
                    else
                        return;
                }
            }
        }
    }
    qCritical() << "response error from" << "\n"
                << "head: " << head << "\n"
                << "body: " << body << "\n"
                << "body size: " << body.size();
    abort();
}

void Client::initRequest(const QString &compile)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_INITIALIZE);
    QString data = setHeader(initialize(d->head.workspace, d->head.language, compile), d->requestIndex);
    qInfo() << "--> server : " << V_INITIALIZE;
    qInfo() << qPrintable(data);
    d->callLanguageRequest(data);
}

void Client::openRequest(const QString &filePath)
{
    QString data = setHeader(lsp::didOpen(filePath)).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDOPEN;
    qInfo() << qPrintable(data);
    d->callLanguageNotify(data);
}

void Client::closeRequest(const QString &filePath)
{
    QString data = setHeader(lsp::didClose(filePath)).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDCLOSE;
    qInfo() << qPrintable(data);
    d->callLanguageNotify(data);
}

void Client::changeRequest(const QString &filePath, const QByteArray &text)
{
    QString data = setHeader(lsp::didChange(filePath, text, d->fileVersion[filePath]));
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDCHANGE;
    qInfo() << qPrintable(data);
    d->callLanguageNotify(data);
}

void Client::symbolRequest(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DOCUMENTSYMBOL);
    QString data = setHeader(symbol(filePath), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DOCUMENTSYMBOL;
    qInfo() << qPrintable(data);
    d->callLanguageRequest(data);
}

void Client::renameRequest(const QString &filePath, const Position &pos, const QString &newName)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_RENAME);
    QString data = setHeader(lsp::rename(filePath, pos, newName), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_RENAME;
    qInfo() << qPrintable(data);
    d->callLanguageRequest(data);
}

void Client::definitionRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DEFINITION);
    QString data = setHeader(lsp::definition(filePath, pos), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DEFINITION;
    qInfo() << qPrintable(data);
    d->callLanguageRequest(data);
}

void Client::completionRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_COMPLETION);
    QString data = setHeader(lsp::completion(filePath, pos), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_COMPLETION;
    qInfo() << qPrintable(data);
    d->callLanguageRequest(data);
}

void Client::signatureHelpRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_SIGNATUREHELP);
    QString data = setHeader(lsp::signatureHelp(filePath, pos), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_SIGNATUREHELP;
    d->callLanguageRequest(data);
}

void Client::referencesRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_REFERENCES);
    QString data = setHeader(lsp::references(filePath, pos), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_REFERENCES;
    d->callLanguageRequest(data);
}

void Client::docHighlightRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT);
    QString data = setHeader(lsp::documentHighlight(filePath, pos), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT;
    qInfo() << qPrintable(data);
    d->callLanguageRequest(data);
}

void Client::docSemanticTokensFull(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_SEMANTICTOKENS + "/full");
    QString data = setHeader(documentSemanticTokensFull(filePath), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_SEMANTICTOKENS + "/full";
    qInfo() << qPrintable(data);
    d->callLanguageRequest(data);
}

void Client::docHoverRequest(const QString &filePath, const Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_HOVER);
    QString data = setHeader(lsp::hover(filePath, pos), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_TEXTDOCUMENT_HOVER;
    qInfo() << qPrintable(data);
    d->callLanguageRequest(data);
}

void Client::shutdownRequest()
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_SHUTDOWN);
    QString data = setHeader(lsp::shutdown(), d->requestIndex).toLatin1();
    qInfo() << "--> server : " << V_SHUTDOWN;
    qInfo() << qPrintable(data);
    d->callLanguageRequest(data);
}

void Client::exitRequest()
{
    QString data = setHeader(exit()).toLatin1();
    qInfo() << "--> server : " << V_SHUTDOWN;
    qInfo() << qPrintable(data);
    d->callLanguageNotify(data);
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
    if (d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_INITIALIZE) {
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

        d->secTokensProvider = provider;

        emit requestResult(provider);
        return true;
    }
    return false;
}

bool Client::openResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_DIDOPEN) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DIDOPEN;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::changeResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_DIDCHANGE) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DOCUMENTSYMBOL;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::symbolResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_DOCUMENTSYMBOL) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DOCUMENTSYMBOL;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::renameResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_RENAME) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_RENAME
                << jsonObj;
        d->requestSave.remove(calledID);
        QJsonObject resultObj = jsonObj.value(K_RESULT).toObject();

        QJsonObject changesObj = resultObj.value("changes").toObject();
        newlsp::WorkspaceEdit workspaceEdit;
        if (!changesObj.isEmpty()) {
            // std::optional<> changes;
            newlsp::WorkspaceEdit::Changes changes;
            for (auto fileKey : changesObj.keys()) {
                auto addionTextEditArray = changesObj[fileKey].toArray();
                std::vector<newlsp::TextEdit> textEdits;
                for (auto addion : addionTextEditArray){
                    auto addionObj = addion.toObject();
                    auto rangeObj = addionObj.value(K_RANGE).toObject();
                    auto startObj = rangeObj.value(K_START).toObject();
                    auto endObj = rangeObj.value(K_END).toObject();
                    std::string newText = addionObj.value(K_NewText).toString().toStdString();
                    newlsp::Position startPos{startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt()};
                    newlsp::Position endPos{endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt()};
                    newlsp::Range range{startPos, endPos};
                    textEdits.push_back(newlsp::TextEdit{range, newText});
                }
                changes[fileKey.toStdString()] = textEdits;
            }
            workspaceEdit.changes = changes;
        }

        QJsonArray documentChangesArray = resultObj.value("documentChanges").toArray();
        if (!documentChangesArray.isEmpty()) {
            newlsp::WorkspaceEdit::DocumentChanges documentChanges;
            std::vector<newlsp::TextDocumentEdit> textDocumentEdits;
            std::vector<newlsp::CreateFile> createFiles;
            std::vector<newlsp::RenameFile> renameFiles;
            std::vector<newlsp::DeleteFile> deleteFiles;
            for (auto one : documentChangesArray) {
                QJsonObject oneObj = one.toObject();
                if (oneObj.contains("edits") && oneObj.contains("textDocument")) { // std::vector<newlsp::TextDocumentEdit>
                    newlsp::TextDocumentEdit textDocumentEdit;

                    newlsp::OptionalVersionedTextDocumentIdentifier ovtdi;
                    QJsonObject textDocumentOneObj = oneObj.value("textDocument").toObject();
                    ovtdi.version = textDocumentOneObj.value(K_VERSION).toInt();
                    ovtdi.uri = textDocumentOneObj.value(K_URI).toString().toStdString();
                    textDocumentEdit.textDocument.version = ovtdi.version;
                    textDocumentEdit.textDocument.uri = ovtdi.uri;

                    std::vector<newlsp::AnnotatedTextEdit> annotatedTextEdits;
                    std::vector<newlsp::TextEdit> textEdits;
                    QJsonArray editsArray = oneObj.value("edits").toArray();
                    for (auto editsOne : editsArray) {
                        QJsonObject editsOneObj = editsOne.toObject();
                        QJsonObject editsOneRangeObj = editsOneObj.value("range").toObject();
                        QJsonObject editsOneRangeStartObj = editsOneRangeObj.value(K_START).toObject();
                        QJsonObject editsOneRangeEndObj = editsOneRangeObj.value(K_END).toObject();
                        std::string editsOneNewText = editsOneObj.value("newText").toString().toStdString();
                        newlsp::Range editsOneRange{
                            newlsp::Position{editsOneRangeStartObj.value(K_LINE).toInt(), editsOneRangeStartObj.value(K_CHARACTER).toInt()},
                            newlsp::Position{editsOneRangeEndObj.value(K_LINE).toInt(), editsOneRangeEndObj.value(K_CHARACTER).toInt()}
                        };
                        if (editsOneObj.contains("annotationId")) { // edits: (TextEdit | AnnotatedTextEdit)[];
                            newlsp::ChangeAnnotationIdentifier changeAnnIdf = editsOneObj.value("annotationId").toString().toStdString();
                            newlsp::AnnotatedTextEdit annotatedTextEdit{};
                            annotatedTextEdit.range = editsOneRange;
                            annotatedTextEdit.newText = editsOneNewText;
                            annotatedTextEdit.annotationId = changeAnnIdf;
                            annotatedTextEdits.push_back(annotatedTextEdit);
                        } else {
                            newlsp::TextEdit textEdit;
                            textEdit.range = editsOneRange;
                            textEdit.newText = editsOneNewText;
                            textEdits.push_back(textEdit);
                        }
                    }
                    if (!annotatedTextEdits.empty()) {
                        textDocumentEdit.edits = annotatedTextEdits;
                    } else if (!textEdits.empty()){
                        textDocumentEdit.edits = textEdits;
                    }
                    textDocumentEdits.push_back(textDocumentEdit);
                } else {
                    QString oneObjKind = oneObj.value("kind").toString();
                    if ("create" == oneObjKind) {
                        newlsp::CreateFile createFile;
                        createFile.uri = oneObj.value("uri").toString().toStdString();
                        QJsonObject oneObjOptions = oneObj.value("options").toObject();
                        if (!oneObjOptions.empty()) {
                            newlsp::CreateFileOptions options;
                            options.overwrite = oneObjOptions.value("overwrite").toBool();
                            options.ignoreIfExists = oneObjOptions.value("ignoreIfExists").toBool();
                            createFile.options = options;
                        }
                        QJsonValue annotationIdJV = oneObj.value("annotationId");
                        if (!annotationIdJV.isNull()) {
                            createFile.annotationId = annotationIdJV.toString().toStdString();
                        }
                        createFiles.push_back(createFile);
                    } else if ("delete" == oneObjKind) {
                        newlsp::DeleteFile deleteFile;
                        deleteFile.uri = oneObj.value("uri").toString().toStdString();
                        QJsonObject oneObjOptions = oneObj.value("options").toObject();
                        if (!oneObjOptions.empty()) {
                            newlsp::DeleteFileOptions options;
                            options.recursive = oneObjOptions.value("recursive").toBool();
                            options.ignoreIfNotExists = oneObjOptions.value("ignoreIfNotExists").toBool();
                            deleteFile.options = options;
                        }
                        QJsonValue annotationIdJV = oneObj.value("annotationId");
                        if (!annotationIdJV.isNull()) {
                            deleteFile.annotationId = annotationIdJV.toString().toStdString();
                        }
                        deleteFiles.push_back(deleteFile);
                    } else if ("rename" == oneObjKind) {
                        newlsp::RenameFile renameFile;
                        renameFile.oldUri = oneObj.value("oldUri").toString().toStdString();
                        renameFile.newUri = oneObj.value("newUri").toString().toStdString();
                        QJsonObject oneObjOptions = oneObj.value("options").toObject();
                        if (!oneObjOptions.empty()) {
                            newlsp::RenameFileOptions options;
                            options.overwrite = oneObjOptions.value("overwrite").toBool();
                            options.ignoreIfExists = oneObjOptions.value("ignoreIfExists").toBool();
                            renameFile.options = options;
                        }
                        QJsonValue annotationIdJV = oneObj.value("annotationId");
                        if (!annotationIdJV.isNull()) {
                            renameFile.annotationId = annotationIdJV.toString().toStdString();
                        }
                        renameFiles.push_back(renameFile);
                    }
                }
                // set workspaceEdit.documentChanges
                if (!textDocumentEdits.empty()) {
                    workspaceEdit.documentChanges = textDocumentEdits;
                } else if (!createFiles.empty()) {
                    workspaceEdit.documentChanges = createFiles;
                } else if (!renameFiles.empty()) {
                    workspaceEdit.documentChanges = renameFiles;
                } else if (!deleteFiles.empty()) {
                    workspaceEdit.documentChanges = deleteFiles;
                }
            }
        }

        QJsonObject changeAnnotationsObj = resultObj.value("changeAnnotations").toObject();
        if (!changeAnnotationsObj.isEmpty()) {
            newlsp::WorkspaceEdit::ChangeAnnotations changeAnnotations;
            for (auto idKey: changeAnnotationsObj.keys()) {
                QJsonObject changeAnnotationObj = changeAnnotationsObj[idKey].toObject();
                newlsp::ChangeAnnotation changeAnnotation;
                std::string label = changeAnnotationObj.value("label").toString().toStdString();
                changeAnnotation.label = label;
                if (changeAnnotationObj.contains("needsConfirmation")) {
                    changeAnnotation.needsConfirmation = changeAnnotationObj.value("needsConfirmation").toBool();
                }
                if (changeAnnotationObj.contains("description")) {
                    changeAnnotation.description = changeAnnotationObj.value("description").toString().toStdString();
                }
                changeAnnotations[idKey.toStdString()] = changeAnnotation;
            }
            workspaceEdit.changeAnnotations = changeAnnotations;
        }
        emit renameRes(workspaceEdit);
        return true;
    }
    return false;
}

bool Client::definitionResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.values().contains(V_TEXTDOCUMENT_DEFINITION)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_DEFINITION) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DEFINITION << jsonObj;
        d->requestSave.remove(calledID);

        QJsonValue resultJV = jsonObj.value(K_RESULT);
        if (resultJV.isArray()) {
            QJsonArray resultArray = resultJV.toArray();
            if (resultArray.count() <= 0){
                return false;
            }
            std::vector<newlsp::Location> locations;
            std::vector<newlsp::LocationLink> locationLinks;
            for (auto one : resultArray) {
                QJsonObject oneObj = one.toObject();
                if (oneObj.contains("range") && oneObj.contains("uri")) {
                    QJsonObject rangeObj = oneObj.value("range").toObject();
                    QJsonObject startObj = rangeObj.value("start").toObject();
                    QJsonObject endObj = rangeObj.value("end").toObject();
                    std::string uri = oneObj.value("uri").toString().toStdString();
                    newlsp::Range range{
                        {startObj.value("line").toInt(), startObj.value("character").toInt()},
                        {endObj.value("line").toInt(), endObj.value("character").toInt()}
                    };
                    locations.push_back({uri, range});
                } else if (oneObj.contains("originSelectionRange")
                           && oneObj.contains("targetUri")
                           && oneObj.contains("targetRange")
                           && oneObj.contains("targetSelectionRange")) {
                    std::string targetUri = oneObj.value("targetUri").toString().toStdString();
                    QJsonObject rangeObj, startObj, endObj;
                    // originSelectionRange
                    rangeObj = oneObj.value("originSelectionRange").toObject();
                    startObj = rangeObj.value("start").toObject();
                    endObj = endObj.value("end").toObject();
                    newlsp::Range originSelectionRange {
                        {startObj.value("line").toInt(), startObj.value("character").toInt()},
                        {endObj.value("line").toInt(), endObj.value("character").toInt()}
                    };
                    // targetRange
                    rangeObj = oneObj.value("targetRange").toObject();
                    startObj = rangeObj.value("start").toObject();
                    endObj = endObj.value("end").toObject();
                    newlsp::Range targetRange {
                        {startObj.value("line").toInt(), startObj.value("character").toInt()},
                        {endObj.value("line").toInt(), endObj.value("character").toInt()}
                    };
                    // targetSelectionRange
                    rangeObj = oneObj.value("targetSelectionRange").toObject();
                    startObj = rangeObj.value("start").toObject();
                    endObj = endObj.value("end").toObject();
                    newlsp::Range targetSelectionRange {
                        {startObj.value("line").toInt(), startObj.value("character").toInt()},
                        {endObj.value("line").toInt(), endObj.value("character").toInt()}
                    };
                    locationLinks.push_back({originSelectionRange, targetUri, targetRange, targetSelectionRange});
                }
            }

            if (!locationLinks.empty()) {
                emit definitionRes(locationLinks); return true;
            } else if (!locations.empty()) {
                emit definitionRes(locations); return true;
            }
        } else if (resultJV.isObject()){
            QJsonObject locationObj = resultJV.toObject();
            if (locationObj.contains("range") && locationObj.contains("uri")) {
                QJsonObject rangeObj = locationObj.value("range").toObject();
                QJsonObject startObj = rangeObj.value("start").toObject();
                QJsonObject endObj = rangeObj.value("end").toObject();
                std::string uri = locationObj.value("uri").toString().toStdString();
                newlsp::Range range{
                    {startObj.value("line").toInt(), startObj.value("character").toInt()},
                    {endObj.value("line").toInt(), endObj.value("character").toInt()}
                };
                emit definitionRes(newlsp::Location{uri, range});
                return true;
            }
        }
    }
    return false;
}

bool Client::completionResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.values().contains(V_TEXTDOCUMENT_COMPLETION)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_COMPLETION) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_COMPLETION /*<< jsonObj*/;
        d->requestSave.remove(calledID);
        QJsonObject resultObj = jsonObj.value(K_RESULT).toObject();
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
    if(d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_SIGNATUREHELP) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_SIGNATUREHELP;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::hoverResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_HOVER) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_HOVER << jsonObj;
        d->requestSave.remove(calledID);
        QJsonObject resultObj = jsonObj.value("result").toObject();
        newlsp::Hover hover;
        QJsonValue contentsJV = resultObj.value("contents");
        if (contentsJV.isArray()) { // to MarkedString[]
            QJsonArray contentsJA = contentsJV.toArray();
            std::vector<newlsp::MarkedString> markedStringVec{};
            for (auto one : contentsJA) {
                if (one.isObject()) {
                    QJsonObject oneObj = one.toObject();
                    markedStringVec.push_back(newlsp::MarkedString{
                                                  oneObj.value("language").toString().toStdString(),
                                                  oneObj.value("value").toString().toStdString()
                                              });
                } else if (one.isString()){
                    markedStringVec.push_back(newlsp::MarkedString{one.toString().toStdString()});
                }
            }
            hover.contents = markedStringVec;
        } else { // MarkedString or MarkupContent
            QJsonObject contentsObj = contentsJV.toObject();
            QJsonValue contents_kind_JV = contentsObj.value("kind");
            QJsonValue contents_value_JV = contentsObj.value("value");
            QJsonValue contents_language_JV = contentsObj.value("language");
            if (!contents_kind_JV.isNull() && !contents_value_JV.isNull()) { // MarkupContent
                hover.contents = newlsp::MarkupContent {
                        contents_kind_JV.toString().toStdString(),
                        contents_value_JV.toString().toStdString() };
            } else if (!contents_language_JV.isNull() && !contents_value_JV.isNull()) { // MarkupString
                hover.contents = newlsp::MarkedString {
                        contents_language_JV.toString().toStdString(),
                        contents_value_JV.toString().toStdString() };
            } else {
                hover.contents = newlsp::MarkedString { contentsJV.toString().toStdString() };
            }
        }
        QJsonValue rangeJV = resultObj.value("range");
        if (!rangeJV.isNull()) {
            QJsonObject rangeObj = rangeJV.toObject();
            QJsonObject startObj = rangeObj.value(K_START).toObject();
            QJsonObject endObj = rangeObj.value(K_END).toObject();
            hover.range = newlsp::Range{
                    newlsp::Position { startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt() },
                    newlsp::Position { endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt() } };
        }
        emit hoverRes(hover);
        return true;
    }
    return false;
}

bool Client::referencesResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_REFERENCES) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_REFERENCES;
        References refs;
        auto resultArray = jsonObj.value(K_RESULT).toArray();
        for (auto item : resultArray) {
            auto itemObj = item.toObject();
            auto rangeObj = itemObj.value(K_RANGE).toObject();
            auto startObj = rangeObj.value(K_START).toObject();
            auto endObj = rangeObj.value(K_END).toObject();
            QString url = itemObj.value(K_URI).toString();
            Location location;
            location.fileUrl = url;
            location.range.start = Position{startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt()};
            location.range.end = Position{endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt()};
            refs << location;
        }
        emit requestResult(refs);
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::docHighlightResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT) {
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
    if(d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_SEMANTICTOKENS + "/full") {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_SEMANTICTOKENS + "full"
                << jsonObj;
        d->requestSave.remove(calledID);

        QJsonObject result = jsonObj.value(K_RESULT).toObject();
        d->semanticTokenResultId = jsonObj.value("resultId").toInt();

        QJsonArray dataArray = result.value(K_DATA).toArray();
        if(dataArray.isEmpty())
            return true;

        QList<lsp::Data> results;
        auto itera = dataArray.begin();
        while (itera != dataArray.end()) {
            results << lsp::Data {
                       Position{itera++->toInt(), itera++->toInt()},
                       int(itera++->toInt()),
                       itera++->toInt(),
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
    if(d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_TEXTDOCUMENT_DIDCLOSE) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_DIDCLOSE;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::exitResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if(d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_EXIT) {
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
    if(d->requestSave.keys().contains(calledID)
            && d->requestSave.value(calledID) == V_SHUTDOWN) {
        qInfo() << "client <-- : " << V_SHUTDOWN
                << jsonObj;
        d->requestSave.remove(calledID);
        return true;
    }
    return false;
}

bool Client::diagnosticsCalled(const QJsonObject &jsonObj)
{
    if (!jsonObj.keys().contains(K_METHOD)
            || jsonObj.value(K_METHOD).toString() != V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS)
        return false;
    qInfo() << "client <-- : " << V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS;

    newlsp::PublishDiagnosticsParams publishDiagnosticsParams;

    QJsonObject paramsObj = jsonObj.value(K_PARAMS).toObject();
    QJsonArray array = paramsObj.value(K_DIAGNOSTICS).toArray();

    for (auto val : array) {
        QJsonObject diagnosticObj = val.toObject();
        QJsonObject rangeObj = diagnosticObj.value(K_RANGE).toObject();
        QJsonObject startObj = rangeObj.value(K_START).toObject();
        QJsonObject endObj = rangeObj.value(K_END).toObject();
        std::vector<newlsp::DiagnosticRelatedInformation> reletedInformation;
        for (auto reInfo : diagnosticObj.value(K_RELATEDINFOMATION).toArray()) {
            auto reInfoObj = reInfo.toObject();
            QJsonObject reInfoLocationObj = reInfoObj.value(K_LOCATION).toObject();
            QJsonObject reInfoLocationRangeObj = reInfoLocationObj.value(K_RANGE).toObject();
            QJsonObject reInfoLocationStartObj = reInfoLocationRangeObj.value(K_START).toObject();
            QJsonObject reInfoLocationEndObj = reInfoLocationRangeObj.value(K_END).toObject();
            std::string reInfoLocationUrl = reInfoLocationObj.value(K_URI).toString().toStdString();
            std::string reInfoMessage = reInfoObj.value(K_MESSAGE).toString().toStdString();
            newlsp::DiagnosticRelatedInformation infomationOne
            {
                newlsp::Location {
                    newlsp::DocumentUri { reInfoLocationUrl },
                    newlsp::Range {
                        { reInfoLocationRangeObj.value(K_LINE).toInt(), reInfoLocationRangeObj.value(K_CHARACTER).toInt()},
                        { reInfoLocationEndObj.value(K_LINE).toInt(), reInfoLocationEndObj.value(K_CHARACTER).toInt()}
                    }
                },
                std::string{
                    reInfoMessage
                }
            };
            reletedInformation.push_back(infomationOne);
        }

        newlsp::Diagnostic diagnostic;
        diagnostic.range = {
            { startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt()},
            { endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt()}
        };

        QJsonValue severityJV = diagnosticObj.value(K_SEVERITY);
        if (!severityJV.isNull()) {
            diagnostic.severity = Diagnostic::Severity(severityJV.toInt());
        }

        QJsonValue codeJV = diagnosticObj.value(K_CODE);
        if (!codeJV.isNull()) {
            if (codeJV.isString()) {
                diagnostic.code = codeJV.toString().toStdString();
            } else {
                diagnostic.code = int(codeJV.toInt());
            }
        }

        QJsonValue codeDescriptionJV = diagnosticObj.value("codeDescription");
        if (!codeDescriptionJV.isNull()) {
            QJsonObject cdObj = codeDescriptionJV.toObject();
            diagnostic.codeDescription = {cdObj.value("href").toString().toStdString()};
        }

        QJsonValue sourceJV =  diagnosticObj.value("source");
        if (!sourceJV.isNull()) {
            diagnostic.source = sourceJV.toString().toStdString();
        }

        QJsonValue messageJV = diagnosticObj.value(K_MESSAGE);
        if (!messageJV.isNull()) {
            diagnostic.message = messageJV.toString().toStdString();
        }

        QJsonValue tagsJV = diagnosticObj.value("tags");
        if (!tagsJV.isNull()) {
            decltype (diagnostic.tags) tags;
            if (tagsJV.isArray()) {
                QJsonArray tagsArray = tagsJV.toArray();
                for (auto one : tagsArray) { tags.value().push_back(one.toInt()); }
            }
            diagnostic.tags = tags;
        }

        if (!reletedInformation.empty()) {
            diagnostic.relatedInformation = reletedInformation;
        }

        QJsonValue dataJV = diagnosticObj.value("data");
        if (!dataJV.isNull()) {
            //nothing to do
        }
        publishDiagnosticsParams.diagnostics.push_back(diagnostic);
    }

    publishDiagnosticsParams.version = paramsObj.value(K_VERSION).toInt();
    publishDiagnosticsParams.uri = paramsObj.value(K_URI).toString().toStdString();
    emit publishDiagnostics(publishDiagnosticsParams);
    return true;
}

bool Client::serverCalled(const QJsonObject &jsonObj)
{
    if (diagnosticsCalled(jsonObj))
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

    if (referencesResult(jsonObj))
        return true;

    if (renameResult(jsonObj))
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

void Client::processJson(const QJsonObject &jsonObj)
{
    if (calledError(jsonObj))
        return;

    if (calledResult(jsonObj))
        return;

    if (serverCalled(jsonObj))
        return;
}

SemanticTokensProvider Client::initSecTokensProvider()
{
    return d->secTokensProvider;
}

QStringList Client::cvtStringList(const QJsonArray &array)
{
    QStringList ret;
    for (auto val : array) {
        ret << val.toString();
    }
    return ret;
}
} // namespace lsp


