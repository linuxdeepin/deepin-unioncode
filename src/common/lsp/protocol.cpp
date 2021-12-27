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

QJsonObject Protocol::initialize(const QString &rootPath)
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
            {"tokenModifiers", QJsonArray{"declaration","definition","readonly","static","deprecated",
                                          "abstract","async","modification","documentation","defaultLibrary"}},
            {"tokenTypes", QJsonArray{"namespace","type","class","enum","interface","struct","typeParameter",
                                      "parameter","variable","property","enumMember","event","function",
                                      "method","macro","keyword","modifier","comment","string","number",
                                      "regexp","operator"}
            }}};

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

QJsonObject Protocol::didOpen(const QString &filePath)
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

QJsonObject Protocol::didChange(const QString &filePath)
{
    //noting to do
    return {};
}

QJsonObject Protocol::didClose(const QString &filePath)
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

QJsonObject Protocol::hover(const QString &filePath, const Protocol::Position &pos)
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

QJsonObject Protocol::definition(const QString &filePath, const Position &pos)
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

QJsonObject Protocol::signatureHelp(const QString &filePath, const Protocol::Position &pos)
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

QJsonObject Protocol::references(const QString &filePath, const Protocol::Position &pos)
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

QJsonObject Protocol::documentHighlight(const QString &filePath, const Protocol::Position &pos)
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

QJsonObject Protocol::shutdown()
{
    QJsonObject shutdown {
        { K_METHOD, V_SHUTDOWN }
    };

    return shutdown;
}

QJsonObject Protocol::exit()
{
    QJsonObject exit {
        { K_METHOD, V_EXIT },
        { K_PARAMS, QJsonValue(QJsonValue::Object)}
    };
    return exit;
}

QJsonObject Protocol::symbol(const QString &filePath)
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

QJsonObject Protocol::completion(const QString &filePath, const Protocol::Position &pos)
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

QString Protocol::setHeader(const QJsonObject &object, int requestIndex)
{
    auto jsonObj = object;
    jsonObj.insert(K_JSON_RPC, V_2_0);
    jsonObj.insert(K_ID, requestIndex);
    QJsonDocument jsonDoc(jsonObj);
    QString jsonStr = jsonDoc.toJson();
    return H_CONTENT_LENGTH + QString(": %0\r\n\r\n").arg(jsonStr.length()) + jsonStr;
}

QString Protocol::setHeader(const QJsonObject &object)
{
    auto jsonObj = object;
    jsonObj.insert(K_JSON_RPC, V_2_0);
    QJsonDocument jsonDoc(jsonObj);
    QString jsonStr = jsonDoc.toJson();
    return H_CONTENT_LENGTH + QString(": %0\r\n\r\n").arg(jsonStr.length()) + jsonStr;
}

bool Protocol::isRequestResult(const QJsonObject &object)
{
    QStringList keys = object.keys();
    if (keys.contains(K_ID) && keys.contains(K_RESULT))
        return true;
    return false;
}

bool Protocol::isRequestError(const QJsonObject &object)
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
};

Client::Client(QObject *parent)
    : QProcess(parent)
    , d (new ClientPrivate)
{
    QObject::connect(this, &QProcess::readyRead,
                     this, &Client::readJson);

    QObject::connect(this, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus exitStatus) {
        qInfo() << "finished" << exitCode << exitStatus;
    });
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
    qInfo() << qPrintable(Protocol::setHeader(Protocol::initialize(rootPath), d->requestIndex).toLatin1());
    write(Protocol::setHeader(Protocol::initialize(rootPath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::openRequest(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DIDOPEN);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDOPEN;
    qInfo() << qPrintable(Protocol::setHeader(Protocol::didOpen(filePath), d->requestIndex).toLatin1());
    write(Protocol::setHeader(Protocol::didOpen(filePath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::closeRequest(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DIDCLOSE);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDCLOSE
            << qPrintable(Protocol::setHeader(Protocol::didClose(filePath), d->requestIndex).toLatin1());
    write(Protocol::setHeader(Protocol::didClose(filePath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::changeRequest(const QString &filePath)
{

}

void Client::symbolRequest(const QString &filePath)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DOCUMENTSYMBOL);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DOCUMENTSYMBOL;
    write(Protocol::setHeader(Protocol::symbol(filePath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::hoverRequest(const QString &filePath, const Protocol::Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_HOVER);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_HOVER;
    write(Protocol::setHeader(Protocol::hover(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::definitionRequest(const QString &filePath, const Protocol::Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DEFINITION);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DEFINITION;
    write(Protocol::setHeader(Protocol::definition(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::completionRequest(const QString &filePath, const Protocol::Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_COMPLETION);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_COMPLETION;
    write(Protocol::setHeader(Protocol::completion(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::signatureHelpRequest(const QString &filePath, const Protocol::Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_SIGNATUREHELP);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_SIGNATUREHELP;
    write(Protocol::setHeader(Protocol::signatureHelp(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::referencesRequest(const QString &filePath, const Protocol::Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_REFERENCES);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_REFERENCES;
    write(Protocol::setHeader(Protocol::references(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::docHighlightRequest(const QString &filePath, const Protocol::Position &pos)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DOCUMENTHIGHLIGHT;
    qInfo() << qPrintable(Protocol::setHeader(Protocol::documentHighlight(filePath, pos), d->requestIndex).toLatin1());
    write(Protocol::setHeader(Protocol::documentHighlight(filePath, pos), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::shutdownRequest()
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_SHUTDOWN);
    qInfo() << "--> server : " << V_SHUTDOWN
            << qPrintable(Protocol::setHeader(Protocol::shutdown(), d->requestIndex).toLatin1());
    write(Protocol::setHeader(Protocol::shutdown(), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::exitRequest()
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_EXIT);
    qInfo() << "--> server : " << V_EXIT
            << qPrintable(Protocol::setHeader(Protocol::exit(), d->requestIndex).toLatin1());
    write(Protocol::setHeader(Protocol::exit(), d->requestIndex).toLatin1());
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
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_COMPLETION;
        d->requestSave.remove(calledID);
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
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_HOVER;
        d->requestSave.remove(calledID);
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
    Protocol::Diagnostics diagnostics;
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
        Protocol::Diagnostic diagnostic
        {
            QString { diagnosticObj.value(K_MESSAGE).toString() },
            Protocol::Range {
                Protocol::Position { startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt()},
                Protocol::Position { endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt()}
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
    QString jsonStrAtOne;
    QList<QJsonObject> jsonObjs;
    char ch;
    while (read(&ch, 1)) {
        jsonStrAtOne += ch;
        if (jsonStrAtOne.contains("\r\n\r\n")) {
            auto contentLength = jsonStrAtOne.split("\r\n\r\n")[0].split(":");
            if (contentLength.size() != 2
                    || contentLength.at(0) != H_CONTENT_LENGTH){
                qCritical() << "Failed, Process Header error"
                            << jsonStrAtOne;
                continue;
            }
            int readCount = contentLength[1].toInt();
            jsonStrAtOne = read(readCount);
            jsonObjs << QJsonDocument::fromJson(jsonStrAtOne.toLatin1()).object();
            jsonStrAtOne.clear();
        }
    }

    foreach (auto jsonObj, jsonObjs) {
        if (calledError(jsonObj))
            continue;

        if (calledResult(jsonObj))
            continue;

        if (serverCalled(jsonObj))
            continue;
    }
}

}
