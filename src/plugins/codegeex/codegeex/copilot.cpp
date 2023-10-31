// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "copilot.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QString>

namespace CodeGeeX {
Copilot::Copilot():manager(new QNetworkAccessManager(this))
{
}

void Copilot::postGenerate(const QString &url, const QString &apiKey, const QString &prompt, const QString &suffix)
{

    QByteArray body = assembleGenerateBody(prompt, suffix, apiKey);
    QNetworkReply *reply = postMessage(url, body);
    processResponse(reply);
}

void Copilot::postComment(const QString &url,
                             const QString &apiKey,
                             const QString &prompt,
                             const QString &lang,
                             const QString &locale,
                             const QString &apisecret)
{
    QByteArray body = assembleCommentBody(prompt, lang, locale, apiKey, apisecret);
    QNetworkReply *reply = postMessage(url, body);
    processResponse(reply);
}

void Copilot::postTranslate(const QString &url,
                               const QString &apiKey,
                               const QString &prompt,
                               const QString &src_lang,
                               const QString &dst_lang,
                               const QString &apisecret)
{
    QByteArray body = assembleTranslateBody(prompt, src_lang, dst_lang, apiKey, apisecret);
    QNetworkReply *reply = postMessage(url, body);
    processResponse(reply);
}

void Copilot::postFixBug(const QString &url,
                            const QString &apiKey,
                            const QString &prompt,
                            const QString &lang,
                            const QString &apisecret)
{
    QByteArray body = assembleBugfixBody(prompt, lang, apiKey, apisecret);
    QNetworkReply *reply = postMessage(url, body);
    processResponse(reply);
}

QNetworkReply *Copilot::postMessage(const QString &url, const QByteArray &body)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    return manager->post(request, body);
}

QByteArray Copilot::assembleGenerateBody(const QString &prompt,
                                     const QString &suffix,
                                     const QString &apikey,
                                     int n,
                                     const QString &apisecret)
{
    QJsonObject json;
    json.insert("prompt", prompt);
    json.insert("isFimEnabled", true);
    json.insert("suffix", suffix);
    json.insert("n", n);
    json.insert("lang", "JavaScript");
    json.insert("apikey", apikey);
    json.insert("apisecret", apisecret);

    QJsonDocument doc(json);
    return doc.toJson();
}

QByteArray Copilot::assembleCommentBody(const QString &prompt, const QString &lang, const QString &locale, const QString &apikey, const QString &apisecret)
{
    QJsonObject json;
    json.insert("prompt", prompt);
    json.insert("lang", lang);
    json.insert("locale", locale);
    json.insert("apikey", apikey);
    json.insert("apisecret", apisecret);

    QJsonDocument doc(json);
    return doc.toJson();
}

QByteArray Copilot::assembleTranslateBody(const QString &prompt, const QString &src_lang, const QString &dst_lang, const QString &apikey, const QString &apisecret)
{
    QJsonObject json;
    json.insert("prompt", prompt);
    json.insert("src_lang", src_lang);
    json.insert("dst_lang", dst_lang);
    json.insert("apikey", apikey);
    json.insert("apisecret", apisecret);

    QJsonDocument doc(json);
    return doc.toJson();
}

QByteArray Copilot::assembleBugfixBody(const QString &prompt,
                                          const QString &lang,
                                          const QString &apikey,
                                          const QString &apisecret)
{
    QJsonObject json;
    json.insert("prompt", prompt);
    json.insert("lang", lang);
    json.insert("apikey", apikey);
    json.insert("apisecret", apisecret);

    QJsonDocument doc(json);
    return doc.toJson();
}

void Copilot::processResponse(QNetworkReply *reply)
{
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qInfo() << "Error:" << reply->errorString();
        } else {
            QString replyMsg = QString::fromUtf8(reply->readAll());
            qInfo() << "Response:" << replyMsg;
            emit response(replyMsg);
        }
    });
}
} // end namespace
