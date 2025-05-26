// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openaicompatiblellm.h"
#include "openaicompatibleconversation.h"

#include <QJsonDocument>
#include <QThread>
#include <QGuiApplication>

QJsonObject parseNonStreamContent(const QByteArray &data)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to parse JSON response";
        return QJsonObject();
    }

    QJsonObject root = jsonDoc.object();
    QJsonObject parseResult;
    if (root.contains("choices") && root["choices"].isArray()) {
        QJsonArray choices = root["choices"].toArray();
        for (const QJsonValue &choice : choices) {
            if (!choice.isObject())
                continue;
            if (choice.toObject().contains("message")) {
                QJsonObject messageObj = choice.toObject()["message"].toObject();
                parseResult["content"] = messageObj["content"].toString();
            } else if (choice.toObject().contains("text")) {
                QString text = choice.toObject()["text"].toString();
                parseResult["content"] = text;
            }
            if (choice.toObject().contains("finish_reason")) {
                 QString reason = choice.toObject()["finish_reason"].toString();
                parseResult["finish_reason"] = reason;
            }
        }
    } else if (root.contains("response")) {
        QString response = root["response"].toString();
        parseResult["content"] = response;
    }
    return parseResult;
}

class OpenAiCompatibleLLMPrivate
{
public:
    OpenAiCompatibleLLMPrivate(OpenAiCompatibleLLM *qq);
    ~OpenAiCompatibleLLMPrivate();

    QNetworkReply *postMessage(const QString &url, const QString &apiKey, const QByteArray &body);
    QNetworkReply *getMessage(const QString &url, const QString &apiKey);
    void replyMessage(const QString &data, AbstractLLM::ResponseState state, AbstractLLM::ResponseHandler handler);
    void processResponse(QNetworkReply *reply, AbstractLLM::ResponseHandler handler = nullptr);
    void handleReplyFinished(QNetworkReply *reply, AbstractLLM::ResponseHandler handler = nullptr);

    QString modelName { "" };
    QString modelPath { "" };
    QString apiKey { "" };
    double temprature { 1.0 };
    int maxTokens = 0;   // default not set
    bool stream { true };

    QByteArray httpResult {};

    OpenAiCompatibleConversation *currentConversation = nullptr;
    QNetworkAccessManager *manager = nullptr;
    OpenAiCompatibleLLM *q = nullptr;
};

OpenAiCompatibleLLMPrivate::OpenAiCompatibleLLMPrivate(OpenAiCompatibleLLM *qq)
    : q(qq)
{
    manager = new QNetworkAccessManager(qq);
    currentConversation = new OpenAiCompatibleConversation();
}

OpenAiCompatibleLLMPrivate::~OpenAiCompatibleLLMPrivate()
{
    if (currentConversation)
        delete currentConversation;
}

QNetworkReply *OpenAiCompatibleLLMPrivate::postMessage(const QString &url, const QString &apiKey, const QByteArray &body)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());

    if (QThread::currentThread() != qApp->thread()) {
        QNetworkAccessManager *threadManager(new QNetworkAccessManager);
        OpenAiCompatibleLLM::connect(QThread::currentThread(), &QThread::finished, threadManager, &QNetworkAccessManager::deleteLater);
        return threadManager->post(request, body);
    }
    return manager->post(request, body);
}

QNetworkReply *OpenAiCompatibleLLMPrivate::getMessage(const QString &url, const QString &apiKey)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());

    if (QThread::currentThread() != qApp->thread()) {
        QNetworkAccessManager *threadManager(new QNetworkAccessManager);
        OpenAiCompatibleLLM::connect(QThread::currentThread(), &QThread::finished, threadManager, &QNetworkAccessManager::deleteLater);
        return threadManager->get(request);
    }
    return manager->get(request);
}

void OpenAiCompatibleLLMPrivate::replyMessage(const QString &data, AbstractLLM::ResponseState state, AbstractLLM::ResponseHandler handler)
{
    if (handler)
        handler(data, state);
    else
        emit q->dataReceived(data, state);
}

void OpenAiCompatibleLLMPrivate::processResponse(QNetworkReply *reply, AbstractLLM::ResponseHandler handler)
{
    OpenAiCompatibleLLM::connect(reply, &QNetworkReply::readyRead, q, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            replyMessage(reply->errorString(), AbstractLLM::ResponseState::Failed, handler);
        } else {
            auto data = reply->readAll();

            // process {"code":,"msg":,"success":false}
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
            if (!jsonDoc.isNull()) {
                QJsonObject jsonObj = jsonDoc.object();
                if (jsonObj.contains("success") && !jsonObj.value("success").toBool()) {
                    replyMessage(jsonObj.value("msg").toString(), AbstractLLM::Failed, handler);
                    return;
                }
            }

            httpResult.append(data);
            QString content;
            QJsonObject retJson;
            if (stream) {
                retJson = OpenAiCompatibleConversation::parseContentString(QString(data));
                if (retJson.contains("content"))
                    content = retJson.value("content").toString();
            } else {
                retJson = parseNonStreamContent(data);
                content = retJson["content"].toString();
            }

            if (retJson["finish_reason"].toString() == "length")
                replyMessage(content, AbstractLLM::ResponseState::CutByLength, handler);
            else if (retJson["finish_reason"].toString() == "stop")
                replyMessage(content, AbstractLLM::Success, handler);
            else
                replyMessage(content, AbstractLLM::Receiving, handler);
        }
    });
}

void OpenAiCompatibleLLMPrivate::handleReplyFinished(QNetworkReply *reply, AbstractLLM::ResponseHandler handler)
{
    if (q->modelState() == AbstractLLM::Idle)   // llm is alread stopped
        return;
    if (reply->error()) {
        qWarning() << "NetWork Error: " << reply->errorString();
        replyMessage(reply->errorString(), AbstractLLM::Failed, handler);
    }
    q->setModelState(AbstractLLM::Idle);
}

OpenAiCompatibleLLM::OpenAiCompatibleLLM(QObject *parent)
    : AbstractLLM(parent), d(new OpenAiCompatibleLLMPrivate(this))
{
}

OpenAiCompatibleLLM::~OpenAiCompatibleLLM()
{
    if (d)
        delete d;
}

QString OpenAiCompatibleLLM::modelName() const
{
    return d->modelName;
}

QString OpenAiCompatibleLLM::modelPath() const
{
    return d->modelPath;
}

Conversation *OpenAiCompatibleLLM::getCurrentConversation()
{
    return d->currentConversation;
}

void OpenAiCompatibleLLM::setModelName(const QString &name)
{
    d->modelName = name;
}

void OpenAiCompatibleLLM::setModelPath(const QString &path)
{
    d->modelPath = path;
}

void OpenAiCompatibleLLM::setApiKey(const QString &key)
{
    d->apiKey = key;
}

bool OpenAiCompatibleLLM::checkValid(QString *errStr)
{
    // Check if the model is valid
    if (d->modelPath.isEmpty()) {
        if (errStr == nullptr)
            return false;
        *errStr = "Model path is empty";
        qWarning() << *errStr;
        return false;
    }

    OpenAiCompatibleConversation c;
    c.setSystemData("You are a test assistant");
    c.addUserData("Testing. Just say hi and nothing else");

    auto obj = create(c);
    QEventLoop loop;
    bool valid = false;
    QString errstr;

    QByteArray body = QJsonDocument(obj).toJson();
    QNetworkReply *reply = d->postMessage(modelPath() + "/chat/completions", d->apiKey, body);
    connect(reply, &QNetworkReply::finished, &loop, [&, this](){
        if (reply->error()) {
            *errStr = reply->errorString();
            valid = false;
        } else {
            valid = true;
        }
        loop.quit();
    });

    loop.exec();
    return valid;
}

QJsonObject OpenAiCompatibleLLM::create(const Conversation &conversation)
{
    QJsonObject dataObject;
    dataObject.insert("model", d->modelName);
    dataObject.insert("messages", conversation.getConversions());
    dataObject.insert("temperature", qBound(0.01, d->temprature, 0.99));
    dataObject.insert("stream", d->stream);
    if (d->maxTokens != 0)
        dataObject.insert("max_tokens", d->maxTokens);

    return dataObject;
}

void OpenAiCompatibleLLM::request(const QJsonObject &data)
{
    setModelState(Busy);
    QByteArray body = QJsonDocument(data).toJson();
    d->httpResult.clear();
    d->currentConversation->update(body);

    QNetworkReply *reply = d->postMessage(modelPath() + "/chat/completions", d->apiKey, body);
    connect(this, &OpenAiCompatibleLLM::requstCancel, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=](){
        if (!d->httpResult.isEmpty())
            d->currentConversation->update(d->httpResult);
        if (reply->error())
            d->currentConversation->popUserData();
        d->handleReplyFinished(reply);
    });

    d->processResponse(reply);
}

void OpenAiCompatibleLLM::request(const QString &prompt, ResponseHandler handler)
{
    setModelState(Busy);
    QJsonObject dataObject;
    dataObject.insert("model", d->modelName);
    dataObject.insert("prompt", prompt);
    dataObject.insert("temperature", qBound(0.01, d->temprature, 0.99));
    dataObject.insert("stream", d->stream);
    if (d->maxTokens != 0)
        dataObject.insert("max_tokens", d->maxTokens);

    QNetworkReply *reply = d->postMessage(modelPath() + "/completions", d->apiKey, QJsonDocument(dataObject).toJson());
    connect(this, &OpenAiCompatibleLLM::requstCancel, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=](){
        d->handleReplyFinished(reply, handler);
    });

    d->processResponse(reply, handler);
}

void OpenAiCompatibleLLM::generate(const QString &prefix, const QString &suffix)
{
    setModelState(Busy);
    QJsonObject dataObject;
    dataObject.insert("model", d->modelName);
    dataObject.insert("suffix", suffix);
    dataObject.insert("prompt", prefix);
    dataObject.insert("temperature", 0.01);
    dataObject.insert("stop", "\n\n");
    dataObject.insert("stream", d->stream);
    if (d->maxTokens != 0)
        dataObject.insert("max_tokens", d->maxTokens);
    else
        dataObject.insert("max_tokens", 128); // quickly response

    QNetworkReply *reply = d->postMessage(modelPath() + "/completions", d->apiKey, QJsonDocument(dataObject).toJson());
    connect(this, &OpenAiCompatibleLLM::requstCancel, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=](){
        d->handleReplyFinished(reply);
    });

    d->processResponse(reply);
}

void OpenAiCompatibleLLM::setTemperature(double temperature)
{
    d->temprature = temperature;
}

void OpenAiCompatibleLLM::setStream(bool isStream)
{
    d->stream = isStream;
}

void OpenAiCompatibleLLM::setLocale(Locale lc)
{
    //todo
    Q_UNUSED(lc);
}

void OpenAiCompatibleLLM::cancel()
{
    setModelState(AbstractLLM::Idle);
    d->httpResult.clear();
    emit requstCancel();
    emit dataReceived("", AbstractLLM::ResponseState::Canceled);
}

void OpenAiCompatibleLLM::setMaxTokens(int maxTokens)
{
    d->maxTokens = maxTokens;
}
