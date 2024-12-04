// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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
            if (choice.toObject().contains("finish_reason"))
                parseResult["finish_reason"] = root["finish_reason"].toString();
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

    QString modelName { "" };
    QString modelPath { "" };
    QString apiKey { "" };
    double temprature { 1.0 };
    int maxTokens = 0; // default not set
    bool stream { true };

    QByteArray httpResult {};
    bool waitingResponse { false };

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
        QNetworkAccessManager* threadManager(new QNetworkAccessManager);
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
        QNetworkAccessManager* threadManager(new QNetworkAccessManager);
        OpenAiCompatibleLLM::connect(QThread::currentThread(), &QThread::finished, threadManager, &QNetworkAccessManager::deleteLater);
        return threadManager->get(request);
    }
    return manager->get(request);
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
        *errStr = "Model path is empty";
        qWarning() << *errStr;
        return false;
    }

    OpenAiCompatibleConversation c;
    c.setSystemData("You are a test assistant");
    c.addUserData("Testing. Just say hi and nothing else");

    auto obj = create(c);
    request(obj);
    QEventLoop loop;
    bool valid = false;
    QString errstr;

    connect(this, &AbstractLLM::dataReceived, &loop, [&, this](const QString & data, ResponseState state){
        if (state == ResponseState::Receiving)
            return;

        if (state == ResponseState::Success) {
            valid = true;
        } else {
            *errStr = data;
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
    QByteArray body = QJsonDocument(data).toJson();
    d->httpResult.clear();
    d->waitingResponse = true;
    d->currentConversation->update(body);

    QNetworkReply *reply = d->postMessage(modelPath() + "/v1/chat/completions", d->apiKey, body);
    connect(this, &OpenAiCompatibleLLM::requstCancel, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=](){
        d->waitingResponse = false;
        if (!d->httpResult.isEmpty())
            d->currentConversation->update(d->httpResult);
        if (reply->error()) {
            qWarning() << "NetWork Error: " << reply->errorString();
            emit dataReceived(reply->errorString(), AbstractLLM::ResponseState::Failed);
            return;
        }
        emit dataReceived("", AbstractLLM::ResponseState::Success);
    });

    processResponse(reply);
}

void OpenAiCompatibleLLM::request(const QString &prompt)
{
    if (d->waitingResponse)
        return;

    d->waitingResponse = true;

    QJsonObject dataObject;
    dataObject.insert("model", d->modelName);
    dataObject.insert("prompt", prompt);
    dataObject.insert("temperature", qBound(0.01, d->temprature, 0.99));
    dataObject.insert("stream", d->stream);
    if (d->maxTokens != 0)
        dataObject.insert("max_tokens", d->maxTokens);

    QNetworkReply *reply = d->postMessage(modelPath() + "/v1/completions", d->apiKey, QJsonDocument(dataObject).toJson());
    connect(this, &OpenAiCompatibleLLM::requstCancel, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=](){
        d->waitingResponse = false;
        if (reply->error()) {
            qWarning() << "NetWork Error: " << reply->errorString();
            emit dataReceived(reply->errorString(), AbstractLLM::ResponseState::Failed);
            return;
        }
        emit dataReceived("", AbstractLLM::ResponseState::Success);
    });

    processResponse(reply);
}

void OpenAiCompatibleLLM::generate(const QString &prompt, const QString &suffix)
{
    if (d->waitingResponse)
        return;

    d->waitingResponse = true;

    QJsonObject dataObject;
    dataObject.insert("model", d->modelName);
    dataObject.insert("suffix", suffix);
    dataObject.insert("prompt", prompt);
    dataObject.insert("temperature", 0.01);
    dataObject.insert("stream", d->stream);
    if (d->maxTokens != 0)
        dataObject.insert("max_tokens", d->maxTokens);

    QNetworkReply *reply = d->postMessage(modelPath() + "/api/generate", d->apiKey, QJsonDocument(dataObject).toJson());
    connect(this, &OpenAiCompatibleLLM::requstCancel, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::finished, this, [=](){
        d->waitingResponse = false;
        if (reply->error()) {
            qWarning() << "NetWork Error: " << reply->errorString();
            emit dataReceived(reply->errorString(), AbstractLLM::ResponseState::Failed);
            return;
        }
        emit dataReceived("", AbstractLLM::ResponseState::Success);
    });

    processResponse(reply);
}

void OpenAiCompatibleLLM::setTemperature(double temperature)
{
    d->temprature = temperature;
}

void OpenAiCompatibleLLM::setStream(bool isStream)
{
    d->stream = isStream;
}

void OpenAiCompatibleLLM::processResponse(QNetworkReply *reply)
{
    connect(reply, &QNetworkReply::readyRead, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            emit dataReceived(reply->errorString(), AbstractLLM::ResponseState::Failed);
        } else {
            auto data = reply->readAll();

            // process {"code":,"msg":,"success":false}
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
            if (!jsonDoc.isNull()) {
                QJsonObject jsonObj = jsonDoc.object();
                if (jsonObj.contains("success") && !jsonObj.value("success").toBool()) {
                    emit dataReceived(jsonObj.value("msg").toString(), AbstractLLM::ResponseState::Failed);
                    return;
                }
            }

            d->httpResult.append(data);
            QString content;
            QJsonObject retJson;
            if (d->stream) {
                retJson = OpenAiCompatibleConversation::parseContentString(QString(data));
                if (retJson.contains("content"))
                    content = retJson.value("content").toString();
            } else {
                retJson = parseNonStreamContent(data);
            }

            if (retJson["finish_reason"].toString() == "length")
                emit dataReceived(content, AbstractLLM::ResponseState::CutByLength);
            else
                emit dataReceived(retJson["content"].toString(), AbstractLLM::ResponseState::Receiving);
        }
    });
}

void OpenAiCompatibleLLM::cancel()
{
    d->waitingResponse = false;
    d->httpResult.clear();
    emit requstCancel();
    emit dataReceived("", AbstractLLM::ResponseState::Canceled);
}

void OpenAiCompatibleLLM::setMaxTokens(int maxTokens)
{
    d->maxTokens = maxTokens;
}
