// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "askapi.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDesktopServices>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace CodeGeeX {

static int kCode_Success = 200;

AskApi::AskApi(QObject *parent)
    : QObject(parent),
      manager(new QNetworkAccessManager(this))

{
}

void AskApi::sendLoginRequest(const QString &sessionId,
                              const QString &machineId,
                              const QString &userId,
                              const QString &env)
{
    QString url = QString("https://codegeex.cn/auth?sessionId=%1&%2=%3&device=%4").arg(sessionId).arg(machineId).arg(userId).arg(env);
    QDesktopServices::openUrl(QUrl(url));
}

void AskApi::logout(const QString &codeToken)
{
    QString url = "https://codegeex.cn/prod/code/oauth/logout";

    QNetworkReply *reply = getMessage(url, codeToken);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = toJsonOBject(reply);
        int code = jsonObject["code"].toInt();
        if (code == kCode_Success) {
            emit loginState(kLoginOut);
        } else {
            qWarning() << "logout failed";
        }
    });
}

void AskApi::sendQueryRequest(const QString &codeToken)
{
    QString url = "https://codegeex.cn/prod/code/oauth/getUserInfo";

    QNetworkReply *reply = getMessage(url, codeToken);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = toJsonOBject(reply);
        int code = jsonObject["code"].toInt();
        if (code == kCode_Success) {
            emit loginState(kLoginSuccess);
        } else {
            emit loginState(kLoginFailed);
        }
    });
}

QJsonArray convertHistoryToJSONArray(const QMultiMap<QString, QString> &history)
{
    QJsonArray jsonArray;

    for (auto it = history.constBegin(); it != history.constEnd(); ++it) {
        const QString &key = it.key();
        const QString &value = it.value();

        QJsonObject jsonObject;
        jsonObject["query"] = key;
        jsonObject["answer"] = value;

        jsonArray.append(jsonObject);
    }

    return jsonArray;
}

void AskApi::postSSEChat(const QString &url,
                         const QString &token,
                         const QString &prompt,
                         const QString &machineId,
                         const QMultiMap<QString, QString> &history,
                         const QString &talkId)
{
    QJsonArray jsonArray = convertHistoryToJSONArray(history);
    QByteArray body = assembleSSEChatBody(prompt, machineId, jsonArray, talkId);
    QNetworkReply *reply = postMessage(url, token, body);
    connect(this, &AskApi::stopReceive, reply, [reply]() {
        reply->close();
    });
    processResponse(reply);
}

void AskApi::postNewSession(const QString &url,
                            const QString &token,
                            const QString &prompt,
                            const QString &talkId)
{
    QByteArray body = assembleNewSessionBody(prompt, talkId);
    QNetworkReply *reply = postMessage(url, token, body);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = toJsonOBject(reply);
        int code = jsonObject["code"].toInt();

        emit sessionCreated(talkId, code == kCode_Success);
    });
}

void AskApi::getSessionList(const QString &url, const QString &token, int pageNumber, int pageSize)
{
    QString urlWithParameter = QString(url + "?pageNum=%1&pageSize=%2").arg(pageNumber).arg(pageSize);
    QNetworkReply *reply = getMessage(urlWithParameter, token);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = toJsonOBject(reply);
        int code = jsonObject["code"].toInt();
        if (code == kCode_Success) {
            QJsonArray listArray = jsonObject.value("data").toObject().value("list").toArray();
            QVector<SessionRecord> records;
            for (int i = 0; i < listArray.size(); ++i) {
                SessionRecord record;
                QJsonObject item = listArray[i].toObject();
                record.talkId = item.value("talkId").toString();
                record.createdTime = item.value("createTime").toString();
                record.prompt = item.value("prompt").toString();

                records.append(record);
            }
            emit getSessionListResult(records);
        }
    });
}

void AskApi::getMessageList(const QString &url, const QString &token, int pageNumber, int pageSize, const QString &talkId)
{
    QString urlWithParameter = QString(url + "?pageNum=%1&pageSize=%2&talkId=%3").arg(pageNumber).arg(pageSize).arg(talkId);
    QNetworkReply *reply = getMessage(urlWithParameter, token);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = toJsonOBject(reply);

        int code = jsonObject["code"].toInt();
        if (code == kCode_Success) {
            QJsonArray listArray = jsonObject.value("data").toObject().value("list").toArray();
            QVector<MessageRecord> records;
            for (int i = 0; i < listArray.size(); ++i) {
                MessageRecord record;
                QJsonObject item = listArray[i].toObject();
                record.input = item.value("prompt").toString();
                record.output = item.value("outputText").toString();

                records.append(record);
            }
            emit getMessageListResult(records);
        }
    });
}

void AskApi::deleteSessions(const QString &url, const QString &token, const QStringList &talkIds)
{
    QByteArray body = assembleDelSessionBody(talkIds);
    QNetworkReply *reply = postMessage(url, token, body);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = toJsonOBject(reply);
        int code = jsonObject["code"].toInt();
        emit sessionDeleted(talkIds, code == kCode_Success);
    });
}

void AskApi::setModel(const QString &model)
{
    this->model = model;
}

QNetworkReply *AskApi::postMessage(const QString &url, const QString &token, const QByteArray &body)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("code-token", token.toUtf8());

    return manager->post(request, body);
}

QNetworkReply *AskApi::getMessage(const QString &url, const QString &token)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("code-token", token.toUtf8());

    return manager->get(request);
}

void AskApi::processResponse(QNetworkReply *reply)
{
    connect(reply, &QNetworkReply::readyRead, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
        } else {
            QString replyMsg = QString::fromUtf8(reply->readAll());
            QStringList lines = replyMsg.split('\n');
            QString data;
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
                } else if (key == "data") { // value of 'data': "data:{"text":"a"}"    but only existed 'text' filed for now
                    index = value.indexOf(':');
                    data = value.mid(index + 2, value.length() - index - 4); // remove " "}

                    emit response(id, data, event);
                }
            }
        }
    });
}

QByteArray AskApi::assembleSSEChatBody(const QString &prompt,
                                       const QString &machineId,
                                       const QJsonArray &history,
                                       const QString &talkId)
{
    QJsonObject jsonObject;
    jsonObject.insert("prompt", prompt);
    jsonObject.insert("machineId", machineId);
    jsonObject.insert("client", "deepin-unioncode");
    jsonObject.insert("history", history);
    //temp  support choose to use later
    jsonObject.insert("locale", "zh");
    jsonObject.insert("model", model);
    if(!talkId.isEmpty())
        jsonObject.insert("talkId", talkId);

    return jsonToByteArray(jsonObject);
}

QByteArray AskApi::assembleNewSessionBody(const QString &prompt,
                                          const QString &talkId)
{
    QJsonObject jsonObject;
    jsonObject.insert("prompt", prompt);
    jsonObject.insert("talkId", talkId);

    return jsonToByteArray(jsonObject);
}

QByteArray AskApi::assembleDelSessionBody(const QStringList &talkIds)
{
    QString ret = "[\n";
    for (auto talkId : talkIds) {
        ret += "\"";
        ret += talkId;
        ret += "\"\n";
    }
    ret += "]";

    return ret.toLatin1();
}

QByteArray AskApi::jsonToByteArray(const QJsonObject &jsonObject)
{
    QJsonDocument doc(jsonObject);
    return doc.toJson();
}

QJsonObject AskApi::toJsonOBject(QNetworkReply *reply)
{
    QString response = QString::fromUtf8(reply->readAll());
    QJsonDocument document = QJsonDocument::fromJson(response.toUtf8());
    return document.object();
}
}   // end namespace
