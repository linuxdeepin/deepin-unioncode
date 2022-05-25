#include "client.h"

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

class ClientPrivate
{
    friend class Client;
    int requestIndex = 0;
    QHash<int, QString> requestSave;
    int semanticTokenResultId = 0;
    QHash<QString, int> fileVersion;
    lsp::SemanticTokensProvider secTokensProvider;
};

Client::Client(QObject *parent)
    : QTcpSocket (parent)
    , d (new ClientPrivate)
{
    qRegisterMetaType<Diagnostics>("Diagnostics");
    qRegisterMetaType<SemanticTokensProvider>("SemanticTokensProvider");
    qRegisterMetaType<Symbols>("Symbols");
    qRegisterMetaType<Locations>("Locations");
    qRegisterMetaType<CompletionProvider>("CompletionProvider");
    qRegisterMetaType<SignatureHelps>("SignatureHelps");
    qRegisterMetaType<Hover>("Hover");
    qRegisterMetaType<Highlights>("Highlights");
    qRegisterMetaType<QList<Data>>("QList<Data>");
    qRegisterMetaType<DefinitionProvider>("DefinitionProvider");
    qRegisterMetaType<DiagnosticsParams>("DiagnosticsParams");
    qRegisterMetaType<Data>("Data");
    qRegisterMetaType<QList<Data>>("QList<Data>");
    qRegisterMetaType<RenameChanges>("RenameChanges");
    qRegisterMetaType<References>("References");
    connectToHost(QHostAddress("127.0.0.1"), 3307);
    if (!waitForConnected(1000)) {
        qCritical() << "can't connect to local, port 3307"
                    << this->errorString()
                    << QTcpSocket::localAddress();
    }
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

void Client::initRequest(const Head &head, const QString &compile)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_INITIALIZE);
    qInfo() << "--> server : " << V_INITIALIZE;
    qInfo() << qPrintable(setHeader(initialize(head.workspace, head.language, compile), d->requestIndex).toLatin1());
    write(setHeader(initialize(head.workspace, head.language, compile), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::openRequest(const QString &filePath)
{
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDOPEN;
    qInfo() << qPrintable(setHeader(didOpen(filePath)).toLatin1());
    write(setHeader(didOpen(filePath), d->requestIndex).toLatin1());
    waitForBytesWritten();
}

void Client::closeRequest(const QString &filePath)
{
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDCLOSE
            << qPrintable(setHeader(didClose(filePath)).toLatin1());
    write(setHeader(didClose(filePath)).toLatin1());
    waitForBytesWritten();
}

void Client::changeRequest(const QString &filePath, const QByteArray &text)
{
    qInfo() << "--> server : " << V_TEXTDOCUMENT_DIDCHANGE
            << qPrintable(setHeader(didChange(filePath, text, d->fileVersion[filePath])));
    write(setHeader(didChange(filePath, text, d->fileVersion[filePath])).toLatin1());
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

void Client::renameRequest(const QString &filePath, const Position &pos, const QString &newName)
{
    d->requestIndex ++;
    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_RENAME);
    qInfo() << "--> server : " << V_TEXTDOCUMENT_RENAME;
    qInfo() << qPrintable(setHeader(rename(filePath, pos, newName), d->requestIndex).toLatin1());
    write(setHeader(rename(filePath, pos, newName), d->requestIndex).toLatin1());
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
    qInfo() << "--> server : " << V_SHUTDOWN
            << qPrintable(setHeader(exit(), d->requestIndex).toLatin1());
    write(setHeader(exit()).toLatin1());
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

        d->secTokensProvider = provider;

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

bool Client::renameResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.values().contains(V_TEXTDOCUMENT_RENAME)
            && d->requestSave.key(V_TEXTDOCUMENT_RENAME) == calledID) {
        qInfo() << "client <-- : " << V_TEXTDOCUMENT_RENAME
                << jsonObj;
        d->requestSave.remove(calledID);
        QJsonObject resultObj = jsonObj.value(K_RESULT).toObject();
        QJsonObject changes = resultObj.value("changes").toObject();
        lsp::RenameChanges changesResult;
        for (auto fileKey : changes.keys()) {
            RenameChange change;
            change.documentUri = fileKey;
            auto addionTextEditArray = changes[fileKey].toArray();
            for (auto addion : addionTextEditArray){
                auto addionObj = addion.toObject();
                auto rangeObj = addionObj.value(K_RANGE).toObject();
                auto startObj = rangeObj.value(K_START).toObject();
                auto endObj = rangeObj.value(K_END).toObject();
                QString newText = addionObj.value(K_NewText).toString();
                Position startPos = {startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt()};
                Position endPos = {endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt()};
                Range range = {startPos, endPos} ;
                change.edits << TextEdit {newText, range};
            }
            changesResult << change;
        }
        emit requestResult(changesResult);
        return true;
    }
    return false;
}

bool Client::definitionResult(const QJsonObject &jsonObj)
{
    auto calledID = jsonObj.value(K_ID).toInt();
    if (d->requestSave.values().contains(V_TEXTDOCUMENT_DEFINITION)
            && d->requestSave.key(V_TEXTDOCUMENT_DEFINITION) == calledID) {
        qInfo() << "client <-- : "
                << V_TEXTDOCUMENT_DEFINITION
                << jsonObj;
        d->requestSave.remove(calledID);
        QJsonArray definitionProviderArray = jsonObj.value(K_RESULT).toArray();
        qInfo() << "definitionProviderArray"
                << definitionProviderArray;
        DefinitionProvider definitionProvider;
        foreach(auto val , definitionProviderArray){
            auto valObj = val.toObject();
            auto rangeObj = valObj.value(K_RANGE).toObject();
            auto startObj = rangeObj.value(K_START).toObject();
            auto endObj = rangeObj.value(K_END).toObject();
            definitionProvider <<  Location { Range {
                                   Position{startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt()},
                                   Position{endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt()}},
                                   QUrl{valObj.value(K_URI).toString()}};
        }
        emit requestResult(definitionProvider);
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
    if(d->requestSave.keys().contains(calledID)
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
    DiagnosticsParams diagnosticsParams;
    if (!jsonObj.keys().contains(K_METHOD)
            || jsonObj.value(K_METHOD).toString() != V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS)
        return false;
    qInfo() << "client <-- : " << V_TEXTDOCUMENT_PUBLISHDIAGNOSTICS;

    QJsonObject paramsObj = jsonObj.value(K_PARAMS).toObject();
    QJsonArray array = paramsObj.value(K_DIAGNOSTICS).toArray();

    for (auto val : array) {
        QJsonObject diagnosticObj = val.toObject();
        QString codeStr = diagnosticObj.value(K_CODE).toString();
        QString messageStr = diagnosticObj.value(K_MESSAGE).toString();
        QJsonObject rangeObj = diagnosticObj.value(K_RANGE).toObject();
        QJsonObject startObj = rangeObj.value(K_START).toObject();
        QJsonObject endObj = rangeObj.value(K_END).toObject();
        QVector<DiagnosticRelatedInformation>  reletedInformation;
        for (auto reInfo : diagnosticObj.value(K_RELATEDINFOMATION).toArray()) {
            auto reInfoObj = reInfo.toObject();
            QJsonObject reInfoLocationObj = reInfoObj.value(K_LOCATION).toObject();
            QJsonObject reInfoLocationRangeObj = reInfoLocationObj.value(K_RANGE).toObject();
            QJsonObject reInfoLocationStartObj = reInfoLocationRangeObj.value(K_START).toObject();
            QJsonObject reInfoLocationEndObj = reInfoLocationRangeObj.value(K_END).toObject();
            QUrl reInfoLocationUrl = reInfoLocationObj.value(K_URI).toString();

            DiagnosticRelatedInformation infomationOne;
            infomationOne.location = {
                Location {
                    Range { Position { reInfoLocationRangeObj.value(K_LINE).toInt(),
                                       reInfoLocationRangeObj.value(K_CHARACTER).toInt()},
                            Position { reInfoLocationEndObj.value(K_LINE).toInt(),
                                       reInfoLocationEndObj.value(K_CHARACTER).toInt()}},
                    QUrl {reInfoLocationUrl}
                }
            };
            infomationOne.message = reInfoObj.value(K_MESSAGE).toString();

            reletedInformation << infomationOne;
        }

        Diagnostic diagnostic;
        diagnostic.code = codeStr;
        diagnostic.message = messageStr;
        diagnostic.range = {
            { startObj.value(K_LINE).toInt(), startObj.value(K_CHARACTER).toInt()},
            { endObj.value(K_LINE).toInt(), endObj.value(K_CHARACTER).toInt()}
        };
        diagnostic.relatedInfomation = reletedInformation;
        diagnostic.severity = Diagnostic::Severity(diagnosticObj.value(K_SEVERITY).toInt());
        diagnostic.source = diagnosticObj.value("source").toString();

        diagnosticsParams.diagnostics << diagnostic;
    }

    diagnosticsParams.version = paramsObj.value(K_VERSION).toInt();
    diagnosticsParams.uri = paramsObj.value(K_URI).toString();
    emit notification(diagnosticsParams);
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

ClientManager *ClientManager::instance()
{
    static ClientManager ins;
    return &ins;
}

void ClientManager::initClient(const Head &head, const QString &complie)
{
    if (head.isValid()) {
        auto client = new Client();
        client->initRequest(head, complie);
        new ClientReadThread(client);
        clients[head] = client;
    }
}

void ClientManager::shutdownClient(const Head &head)
{
    auto client = clients[head];
    if (client) {
        client->shutdownRequest();
        client->exitRequest();
        clients.remove(head);
    }
}

Client *ClientManager::get(const Head &head)
{
    return clients[head];
}

class ClientReadThreadPrivate
{
    friend class ClientReadThread;
    Client *client;
    QByteArray array;
    bool stopFlag = false;
};

ClientReadThread::ClientReadThread(Client *client)
    : d(new ClientReadThreadPrivate())
{
    d->client = client;
    client->moveToThread(this);
    QObject::connect(client, &QObject::destroyed,[=]() {
        this->stop();
        this->wait(3000);
        delete this;
    });
    start();
}

void ClientReadThread::stop()
{
    d->stopFlag = true;
}

void ClientReadThread::run()
{
    if (!d->client)
        return;

    while (!d->stopFlag) {
        d->client->waitForReadyRead(3000);
        d->array += d->client->readAll();
        while (d->array.startsWith("Content-Length:")) {
            int index = d->array.indexOf("\r\n\r\n");
            QByteArray head = d->array.mid(0, index);
            QList<QByteArray> headList = head.split(':');
            int readCount = headList[1].toInt();
            QByteArray data = d->array.mid(head.size() + 4, readCount);
            if (data.size() < readCount) {
                break;
            } else if (data.size() > readCount) {
                qCritical() << "process data error";
                abort();
            } else {
                QJsonParseError err;
                QJsonObject jsonObj = QJsonDocument::fromJson(data, &err).object();
                if (jsonObj.isEmpty()) {
                    qCritical() << err.errorString() << qPrintable(data);
                    abort();
                }
                d->client->processJson(jsonObj);
                d->array.remove(0, index);
                d->array.remove(0, readCount + 4);
            }
        }
        int headIndex = d->array.indexOf("Content-Length:");
        if(headIndex > 0) {
            d->array.remove(0, headIndex);
        }
    }
}

} // namespace lsp

bool Head::isValid() const
{
    return !workspace.isEmpty()
            && !language.isEmpty();
}

Head::Head(){}

Head::Head(const QString &workspace, const QString &language)
    : workspace(workspace)
    , language(language)
{
}

Head::Head(const Head &head)
    : workspace(head.workspace)
    , language(head.language)
{
}

uint qHash(const Head &key, uint seed)
{
    return qHash(key.workspace + key.language, seed);
}

bool operator ==(const Head &t1, const Head &t2)
{
    return t1.workspace == t2.workspace
            && t2.language == t2.language;
}
