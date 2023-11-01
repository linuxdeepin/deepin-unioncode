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

AskApi::AskApi(QObject *parent) :
    QObject(parent),
    manager(new QNetworkAccessManager(this))

{
}

void AskApi::sendLoginRequest(const QString &sessionId,
                              const QString &machineId,
                              const QString &userId,
                              const QString &env)
{
    QString url = QString("https://codegeex.cn/auth?sessionId=%1&%2=%3&device=%4").\
            arg(sessionId).arg(machineId).arg(userId).arg(env);
    QDesktopServices::openUrl(QUrl(url));
}

void AskApi::sendQueryRequest(const QString &codeToken)
{
    QString url = "https://codegeex.cn/prod/code/oauth/getUserInfo";

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("code-token", codeToken.toUtf8());

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
        } else {
            QString response = QString::fromUtf8(reply->readAll());
            QJsonDocument document = QJsonDocument::fromJson(response.toUtf8());
            QJsonObject jsonObject = document.object();
            int code = jsonObject["code"].toInt();
            if (code == 401) {
                emit loginState(kLoginFailed);
            } else if (code == 200) {
                emit loginState(kLoginSuccess);
            }
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
                         const QMultiMap<QString, QString> &history)
{
    QJsonArray jsonArray = convertHistoryToJSONArray(history);
    QByteArray body = assembleSSEChatBody(prompt, machineId, jsonArray);
    QNetworkReply *reply = postMessage(url, token, body);
    processResponse(reply);
}

void AskApi::postNewSession(const QString &url,
                            const QString &token,
                            const QString &prompt,
                            const QString &taskId)
{
    QByteArray body = assembleNewSessionBody(prompt, taskId);
    QNetworkReply *reply = postMessage(url, token, body);
    processResponse(reply);
}

QNetworkReply *AskApi::postMessage(const QString &url, const QString &token, const QByteArray &body)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("code-token", token.toUtf8());

    return manager->post(request, body);
}

void AskApi::processResponse(QNetworkReply *reply)
{
    connect(reply, &QNetworkReply::readyRead, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
        } else {
            QString replyMsg = QString::fromUtf8(reply->readAll());
            QStringList lines = replyMsg.split('\n');
            QByteArray data;
            QString event;
            for (const auto& line : lines) {
                if (line.startsWith("event:add")) {
                    event = "add";
                } else if (line.startsWith("event:finish")) {
                    event = "finish";
                } else if (line.startsWith("id:")) {
                    continue;
                } else if (line.startsWith("data:")) {
                    data += line.mid(5) + '\n';
                } else if (line == "") {
                    emit response(data, event);
                    data.clear();
                }
            }
        }
    });
}

QByteArray AskApi::assembleSSEChatBody(const QString &prompt,
                                       const QString &machineId,
                                       const QJsonArray &history)
{
    QJsonObject jsonObject;
    jsonObject.insert("prompt", prompt);
    jsonObject.insert("machineId", machineId);
    jsonObject.insert("history", history);

    return jsonToByteArray(jsonObject);
}

QByteArray AskApi::assembleNewSessionBody(const QString &prompt,
                                          const QString &taskId)
{
    QJsonObject jsonObject;
    jsonObject.insert("prompt", prompt);
    jsonObject.insert("taskid", taskId);

    return jsonToByteArray(jsonObject);
}

QByteArray AskApi::jsonToByteArray(const QJsonObject &jsonObject)
{
    QJsonDocument doc(jsonObject);
    return doc.toJson();
}
} // end namespace
