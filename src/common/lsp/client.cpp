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

namespace lsp {

static QMutex mutex;

static void readForThread(Client *client)
{
    if (!client)
        return;

    qInfo() << "bytesAvailable: " << client->bytesAvailable();
    QString head;
    while (client->bytesAvailable()) {
        head += client->read(1);
        if (head.contains("\r\n\r\n")) {
            auto list = head.split("\r\n\r\n");
            auto contentLength = list[0].split(":");
            if (contentLength.size() != 2) {
                qCritical() << "contentLength error, count != 2";
                qCritical() << "\n********Content LSP**********\n";
                qCritical() << head;
                qCritical() << "\n";
                abort();
            }
            int currentCount = contentLength[1].toInt();
            QString byteArrary = client->read(currentCount);
            while(byteArrary.toUtf8().size() < currentCount) {
                while(!client->waitForReadyRead());
                byteArrary += client->read(currentCount - byteArrary.toUtf8().size());
            }
            qInfo() << "contentLength:\n" << contentLength;
            qInfo() << "#####################";
            qInfo() << "toUtf8()" << byteArrary.toUtf8().size();
            qInfo() << "toLocal8Bit()"<< byteArrary.toLocal8Bit().size();
            qInfo() << "toLatin1()"<< byteArrary.toLatin1().size();
            if (byteArrary.toUtf8().size() == currentCount) {
                qInfo() << "**********************"
                        << qPrintable(byteArrary);
                client->processJson(QJsonDocument::fromJson(byteArrary.toUtf8()).object());
                head.clear();
                qInfo() << "Remaining unread bytes: " << client->bytesAvailable();
                continue;
            } else {
                qCritical() << "Failed, read json error";
                abort();
            }
        }
    }
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
    qRegisterMetaType<Diagnostics>("Diagnostics");
    qRegisterMetaType<SemanticTokensProvider>("SemanticTokensProvider");
    qRegisterMetaType<Symbols>("Symbols");
    qRegisterMetaType<Locations>("Locations");
    qRegisterMetaType<CompletionProvider>("CompletionProvider");
    qRegisterMetaType<SignatureHelps>("SignatureHelps");
    qRegisterMetaType<Hover>("Hover");
    qRegisterMetaType<Highlights>("Highlights");
    qRegisterMetaType<QList<Data>>("QList<Data>");

    // 每一个读任务都开启一个线程处理
    QObject::connect(this, &Client::readyRead, this, [=](){
        QtConcurrent::run([=](){readForThread(this);});
    }, Qt::QueuedConnection);
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
    //    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DIDOPEN);
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
    //    d->requestSave.insert(d->requestIndex, V_TEXTDOCUMENT_DIDCHANGE);
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
    if(d->requestSave.keys().contains(calledID)) {
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

} // namespace lsp
