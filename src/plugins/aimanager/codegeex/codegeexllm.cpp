// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexllm.h"
#include "codegeexconversation.h"
#include "services/option/optionmanager.h"
#include "services/window/windowservice.h"
#include "services/editor/editorservice.h"
#include "services/project/projectservice.h"

#include <QJsonDocument>
#include <QThread>
#include <QGuiApplication>
#include <QUuid>
#include <QDesktopServices>

using namespace dpfservice;

static int kCode_Success = 200;
static const char *kUrlGenerateMultiLine = "https://api.codegeex.cn:8443/v3/completions/inline?stream=false";
static const char *kUrlCreateNewSession = "https://codegeex.cn/prod/code/chatGlmTalk/insert";
static const char *kUrlQueryUserInfo = "https://codegeex.cn/prod/code/oauth/getUserInfo";

QString uuid()
{
    QUuid uuid = QUuid::createUuid();
    return uuid.toString().replace("{", "").replace("}", "").replace("-", "");
}

QPair<QString, QString> getCurrentFileInfo()
{
    EditorService *editorService = dpfGetService(EditorService);
    auto filePath = editorService->currentFile();
    QString fileName;
    if (QFileInfo(filePath).exists())
        fileName = QFileInfo(filePath).fileName();
    else
        fileName = "main.cpp";
    auto fileType = support_file::Language::id(filePath);
    auto fileLang = support_file::Language::idAlias(fileType);

    // The above LANGUAGE class supports fewer file languages, and unknown file languages are temporarily represented by suffix.
    if (fileLang.isEmpty())
        fileLang = QFileInfo(filePath).suffix();
    return qMakePair(fileName, fileLang);
}

class CodeGeeXLLMPrivate
{
public:
    CodeGeeXLLMPrivate(CodeGeeXLLM *qq);
    ~CodeGeeXLLMPrivate();

    QNetworkReply *postMessage(const QString &url, const QString &apiKey, const QByteArray &body);
    QNetworkReply *getMessage(const QString &url, const QString &apiKey);

    void login();
    void saveConfig(const QString &sessionId);
    void loadConfig();
    Entry processJsonObject(const QString &event, QJsonObject *obj);
    void createNewSession();
    void handleReplyFinished(QNetworkReply *reply);
    void handleStreamResponse(const QByteArray &data, AbstractLLM::ResponseHandler handler = nullptr);
    void handleNonStreamResponse(const QByteArray &data, AbstractLLM::ResponseHandler handler = nullptr);
    void replyMessage(const QString &data, AbstractLLM::ResponseState state, AbstractLLM::ResponseHandler handler);
    void processResponse(QNetworkReply *reply, AbstractLLM::ResponseHandler handler = nullptr);

    QString modelName { "" };
    QString modelPath { "" };
    QString apiKey { "" };  // == session id
    QString talkId { "" };
    QString locale { "zh" };
    double temprature { 1.0 };
    int maxTokens = 0; // default not set
    bool stream { true };

    CodeGeeXConversation *c;

    QNetworkAccessManager *manager = nullptr;
    CodeGeeXLLM *q = nullptr;
};

CodeGeeXLLMPrivate::CodeGeeXLLMPrivate(CodeGeeXLLM *qq)
 : q(qq)
{
    c = new CodeGeeXConversation;
    manager = new QNetworkAccessManager(qq);
    loadConfig();
}

CodeGeeXLLMPrivate::~CodeGeeXLLMPrivate()
{
    delete c;
}

QNetworkReply *CodeGeeXLLMPrivate::postMessage(const QString &url, const QString &apiKey, const QByteArray &body)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("code-token", apiKey.toUtf8());

    if (QThread::currentThread() != qApp->thread()) {
        QNetworkAccessManager* threadManager(new QNetworkAccessManager);
        CodeGeeXLLM::connect(QThread::currentThread(), &QThread::finished, threadManager, &QNetworkAccessManager::deleteLater);
        return threadManager->post(request, body);
    }
    return manager->post(request, body);
}

QNetworkReply *CodeGeeXLLMPrivate::getMessage(const QString &url, const QString &apiKey)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("code-token", apiKey.toUtf8());

    if (QThread::currentThread() != qApp->thread()) {
        QNetworkAccessManager* threadManager(new QNetworkAccessManager);
        CodeGeeXLLM::connect(QThread::currentThread(), &QThread::finished, threadManager, &QNetworkAccessManager::deleteLater);
        return threadManager->get(request);
    }
    return manager->get(request);
}

void CodeGeeXLLMPrivate::login()
{
    apiKey = uuid();
    QString machineId = QSysInfo::machineUniqueId();

    QString url = QString("https://codegeex.cn/auth?sessionId=%1&%2=%3&device=%4").arg(apiKey).arg(machineId).arg(apiKey).arg("deepin-unioncode");
    QDesktopServices::openUrl(QUrl(url));
    saveConfig(apiKey);
}

void CodeGeeXLLMPrivate::saveConfig(const QString &sessionId)
{
    QVariantMap map {{ "sessionId", sessionId }};

    OptionManager::getInstance()->setValue("CodeGeeX", "Id", map);
}

void CodeGeeXLLMPrivate::loadConfig()
{
    const auto map = OptionManager::getInstance()->getValue("CodeGeeX", "Id").toMap();
    if (map.isEmpty())
        return;

    apiKey = map.value("sessionId").toString();
}

void CodeGeeXLLMPrivate::createNewSession()
{
    loadConfig(); // incase sessionId has updated
    QString url = kUrlCreateNewSession;
    QString currentMSecsStr = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString sessionTitle("Session_" + currentMSecsStr);
    QString taskId(uuid());

    QJsonObject jsonObject;
    jsonObject.insert("prompt", sessionTitle);
    jsonObject.insert("talkId", taskId);

    QNetworkReply *reply = postMessage(url, apiKey, QJsonDocument(jsonObject).toJson());
    QEventLoop loop;
    CodeGeeXLLM::connect(reply, &QNetworkReply::finished, q, [=, &loop]() {
        if (reply->error()) {
            qCritical() << "CodeGeeX Session created faield \nError:" << reply->errorString();
            loop.exit();
            return;
        }

        QString response = QString::fromUtf8(reply->readAll());
        QJsonDocument document = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject jsonObject = document.object();
        int code = jsonObject["code"].toInt();
        if (code == kCode_Success)
            talkId = taskId;
        loop.exit();
    });
    loop.exec();
}

void CodeGeeXLLMPrivate::handleReplyFinished(QNetworkReply *reply)
{
    if (q->modelState() == AbstractLLM::Idle)   // llm is alread stopped
        return;
    if (reply->error()) {
        qWarning() << "NetWork Error: " << reply->errorString();
        emit q->dataReceived(reply->errorString(), AbstractLLM::ResponseState::Failed);
    }
    q->setModelState(AbstractLLM::Idle);
}

void CodeGeeXLLMPrivate::handleStreamResponse(const QByteArray &data, AbstractLLM::ResponseHandler handler)
{
    QString replyMsg = QString::fromUtf8(data);
    QStringList lines = replyMsg.split('\n');
    QString event;
    QString id;

    for (const auto &line : lines) {
        auto index = line.indexOf(':');
        auto key = line.mid(0, index);
        auto value = line.mid(index + 1);

        if (key == "event") {
            event = value.trimmed();
        } else if (key == "id") {
            id = value.trimmed();
        } else if (key == "data") {
            QJsonParseError error;
            QJsonDocument jsonDocument = QJsonDocument::fromJson(value.toUtf8(), &error);

            QJsonObject jsonObject = jsonDocument.object();
            auto entry = processJsonObject(event, &jsonObject);
            if (error.error != QJsonParseError::NoError) {
                qCritical() << "JSON parse error: " << error.errorString();
                if (event == "finish") {
                    replyMessage(entry.text, AbstractLLM::Failed, handler);
                    return;
                }
                continue;
            }

            if (entry.type == "crawl")
                emit q->customDataReceived("crawl", entry.websites);

            if (event == "add")
                replyMessage(entry.text, AbstractLLM::Receiving, handler);
            else if (event == "finish") {
                c->addResponse(entry.text);
                replyMessage(entry.text, AbstractLLM::Success, handler);
            }
        }
    }
}

void CodeGeeXLLMPrivate::handleNonStreamResponse(const QByteArray &data, AbstractLLM::ResponseHandler handler)
{
    QString replyMsg = QString::fromUtf8(data);
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(replyMsg.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCritical() << "JSON parse error: " << error.errorString();
        replyMessage(error.errorString(), AbstractLLM::Failed, handler);
        return;
    }
    QJsonObject jsonObject = jsonDocument.object();
    if (!jsonObject.value("inline_completions").isUndefined()) {
        auto content = jsonObject.value("inline_completions").toArray().at(0).toObject();
        QString code = content.value("text").toString();
        if (content.value("finish_reason").toString() == "length") {
            // Due to the length limit of the code, the last line will be discarded when the code is truncated.
            auto codeLines = code.split('\n');
            if (codeLines.size() > 1)
                codeLines.removeLast();
            code = codeLines.join('\n');
        }

        replyMessage(code, AbstractLLM::Success, handler);
        return;
    }

    auto response = jsonObject.value("text").toString();
    replyMessage(response, AbstractLLM::Success, handler);
}

Entry CodeGeeXLLMPrivate::processJsonObject(const QString &event, QJsonObject *obj)
{
    Entry entry;
    if (!obj || obj->isEmpty())
        return entry;

    if (event == "add") {
        entry.type = "text";
        entry.text = obj->value("text").toString();
        return entry;
    }

    if (event == "processing") {
        auto type = obj->value("type").toString();
        entry.type = type;
        if (type == "keyword") {
            auto keyWords = obj->value("data").toArray();
            QString keys;
            for (auto key : keyWords)
                keys = keys + key.toString() + " ";
            entry.text = keys.trimmed();
        } else if (type == "crawl") {
            auto crawlObj = obj->value("data").toObject();
            entry.websites = crawlObj;
        }
        return entry;
    }

    if (event == "finish") {
        entry.text = obj->value("text").toString();
        entry.type = event;
    }

    return entry;
}

void CodeGeeXLLMPrivate::replyMessage(const QString &data, AbstractLLM::ResponseState state, AbstractLLM::ResponseHandler handler)
{
    if (handler)
        handler(data, state);
    else
        emit q->dataReceived(data, state);
}

void CodeGeeXLLMPrivate::processResponse(QNetworkReply *reply, AbstractLLM::ResponseHandler handler)
{
    CodeGeeXLLM::connect(reply, &QNetworkReply::readyRead, q, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            replyMessage(reply->errorString(), AbstractLLM::Failed, handler);            
        } else {
            if (stream)
                handleStreamResponse(reply->readAll(), handler);
            else
                handleNonStreamResponse(reply->readAll(), handler);
        }
    });
}

CodeGeeXLLM::CodeGeeXLLM(QObject *parent)
    : AbstractLLM(parent), d(new CodeGeeXLLMPrivate(this))
{
}

CodeGeeXLLM::~CodeGeeXLLM()
{
    delete d;
}

QString CodeGeeXLLM::modelName() const
{
    return d->modelName;
}

QString CodeGeeXLLM::modelPath() const
{
    return d->modelPath;
}

Conversation *CodeGeeXLLM::getCurrentConversation()
{
    return d->c;
}

void CodeGeeXLLM::setModelName(const QString &name)
{
    d->modelName = name;
}

void CodeGeeXLLM::setModelPath(const QString &path)
{
    d->modelPath = path;
}

void CodeGeeXLLM::setApiKey(const QString &key)
{
    d->apiKey = key;
}

bool CodeGeeXLLM::checkValid(QString *errStr)
{
    d->loadConfig();
    QString url = kUrlQueryUserInfo;
    QNetworkReply *reply = d->getMessage(url, d->apiKey);
    QEventLoop loop;
    bool valid = false;
    connect(reply, &QNetworkReply::finished, this, [=, &loop, &valid]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            loop.exit();
            return;
        }
        QString response = QString::fromUtf8(reply->readAll());
        QJsonDocument document = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject jsonObject = document.object();
        int code = jsonObject["code"].toInt();
        if (code == kCode_Success) {
            valid = true;
        } else {
            valid = false;
            *errStr = "Please Login CodeGeeX first";

            QStringList actions { "codegeex_login_default", tr("Login") };
            auto windowService = dpfGetService(WindowService);
            windowService->notifyWithCallback(0, "CodeGeeX", tr("Please login to use CodeGeeX."), actions, [=](const QString &actId) {
                if (actId == "codegeex_login_default")
                    d->login();
            });
        }
        loop.exit();
    });
    loop.exec();

    return valid;
}

QJsonObject CodeGeeXLLM::create(const Conversation &conversation)
{
    QJsonObject dataObject;
    dataObject.insert("ide", qApp->applicationName());

    if (d->talkId.isEmpty())
        d->createNewSession();

    const QJsonArray &array = conversation.getConversions();
    QString prompt = "";
    if (!array.isEmpty() && array.last()["role"] == "user") {
        prompt = array.last()["content"].toString();
    }
    QJsonArray history {}; // [{user}, {assistant} {..}]
    for (int i = 0; i < array.size() - 1; i++) {
        QJsonObject obj;
        if (array[i]["role"] == "user" && array[i+1]["role"] == "assistant") {
            obj.insert("query", array[i]["content"].toString());
            obj.insert("answer", array[i+1]["content"].toString());
        }
        history.append(obj);
    }

    dataObject.insert("prompt", prompt);
    dataObject.insert("machineId",  QString(QSysInfo::machineUniqueId()));
    dataObject.insert("history", history);
    dataObject.insert("locale", d->locale);
    dataObject.insert("model", d->modelName);
    dataObject.insert("stream", d->stream);
    dataObject.insert("talkId", d->talkId);

    return dataObject;
}

void CodeGeeXLLM::request(const QJsonObject &data)
{
    QByteArray body = QJsonDocument(data).toJson();
    setModelState(Busy);

    QNetworkReply *reply = d->postMessage(modelPath(), d->apiKey, body);
    connect(this, &CodeGeeXLLM::requstCancel, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=](){
        d->handleReplyFinished(reply);
    });

    d->processResponse(reply);
}

void CodeGeeXLLM::request(const QString &prompt, ResponseHandler handler)
{
    if (d->talkId.isEmpty())
        d->createNewSession();
    setModelState(Busy);

    QJsonObject dataObject;
    dataObject.insert("ide", qApp->applicationName());
    dataObject.insert("prompt", prompt);
    dataObject.insert("machineId",  QString(QSysInfo::machineUniqueId()));
    dataObject.insert("locale", d->locale);
    dataObject.insert("model", d->modelName);
    dataObject.insert("stream", d->stream);
    dataObject.insert("talkId", d->talkId);

    QNetworkReply *reply = d->postMessage(modelPath(), d->apiKey, QJsonDocument(dataObject).toJson());
    connect(this, &CodeGeeXLLM::requstCancel, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=](){
        d->handleReplyFinished(reply);
    });

    d->processResponse(reply, handler);
}

/*
    data = {
        "context": [{
            "kind":
            "active_document":{}
            },
            {
            "kind":
            "document":{}
            },
            ],
        "model":,
        "lang":
    }
*/
void CodeGeeXLLM::generate(const QString &prefix, const QString &suffix)
{
    setModelState(Busy);

    auto file = getCurrentFileInfo();

    QJsonObject activeDocument;
    activeDocument.insert("path", file.first);
    activeDocument.insert("prefix", prefix);
    activeDocument.insert("suffix", suffix);
    activeDocument.insert("lang", file.second);

    QJsonObject activeContextItem;
    activeContextItem.insert("kind", "active_document");
    activeContextItem.insert("active_document", activeDocument);

//    ProjectService *prjSrv = dpfGetService(ProjectService);
    QJsonArray context;
    context.append(activeContextItem);
//    QJsonObject queryResults = CodeGeeXManager::instance()->query(prjSrv->getActiveProjectInfo().workspaceFolder(), prefix, 5);
//    QJsonArray chunks = queryResults["Chunks"].toArray();

//    for (auto chunk : chunks) {
//        QJsonObject document;
//        document.insert("path", chunk.toObject()["fileName"].toString());
//        document.insert("text", chunk.toObject()["content"].toString());
//        document.insert("lang", file.second);

//        QJsonObject contextItem;
//        contextItem.insert("kind", "document");
//        contextItem.insert("document", document);
//        context.append(contextItem);
//    }

    QJsonObject json;
    json.insert("ide", qApp->applicationName());
    json.insert("ide_version", version());
    json.insert("context", context);
    json.insert("model", modelName());
    json.insert("lang", file.second);
    json.insert("max_new_tokens", d->maxTokens == 0 ? 126 : d->maxTokens);

    QNetworkReply *reply = d->postMessage(kUrlGenerateMultiLine, d->apiKey, QJsonDocument(json).toJson());
    connect(this, &CodeGeeXLLM::requstCancel, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=](){
        d->handleReplyFinished(reply);
    });

    d->processResponse(reply);
}

void CodeGeeXLLM::setTemperature(double temperature)
{
    d->temprature = temperature;
}

void CodeGeeXLLM::setStream(bool isStream)
{
    d->stream = isStream;
}

void CodeGeeXLLM::setLocale(Locale lc)
{
    d->locale = lc == Locale::Zh ? "zh" : "en";
}

void CodeGeeXLLM::cancel()
{
    setModelState(Idle);
    emit requstCancel();
    emit dataReceived("", AbstractLLM::ResponseState::Canceled);
}

void CodeGeeXLLM::setMaxTokens(int maxTokens)
{
    d->maxTokens = maxTokens;
}
