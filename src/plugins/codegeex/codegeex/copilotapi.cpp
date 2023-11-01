// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "copilotapi.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QString>
#include <QHash>


namespace CodeGeeX {
QHash<QString, CopilotApi::ResponseType> kResponseTypes{
    {"multilingual_code_generate", CopilotApi::multilingual_code_generate},
    {"multilingual_code_explain", CopilotApi::multilingual_code_explain},
    {"multilingual_code_translate", CopilotApi::multilingual_code_translate},
    {"multilingual_code_bugfix", CopilotApi::multilingual_code_bugfix}
};
CopilotApi::CopilotApi(QObject *parent)
    : QObject (parent)
    , manager(new QNetworkAccessManager(this))
{
}

void CopilotApi::postGenerate(const QString &url, const QString &apiKey, const QString &prompt, const QString &suffix)
{

    QByteArray body = assembleGenerateBody(prompt, suffix, apiKey);
    QNetworkReply *reply = postMessage(url, body);
    processResponse(reply);
}

void CopilotApi::postComment(const QString &url,
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

void CopilotApi::postTranslate(const QString &url,
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

void CopilotApi::postFixBug(const QString &url,
                            const QString &apiKey,
                            const QString &prompt,
                            const QString &lang,
                            const QString &apisecret)
{
    QByteArray body = assembleBugfixBody(prompt, lang, apiKey, apisecret);
    QNetworkReply *reply = postMessage(url, body);
    processResponse(reply);
}

QNetworkReply *CopilotApi::postMessage(const QString &url, const QByteArray &body)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    return manager->post(request, body);
}

QByteArray CopilotApi::assembleGenerateBody(const QString &prompt,
                                     const QString &suffix,
                                     const QString &apikey,
                                     int n,
                                     const QString &apisecret)
{
    QJsonObject json;
    json.insert("prompt", prompt);
    json.insert("isFimEnabled", false);
    json.insert("suffix", suffix);
    json.insert("n", n);
    json.insert("lang", "c++");
    json.insert("apikey", apikey);
    json.insert("apisecret", apisecret);

    QJsonDocument doc(json);
    return doc.toJson();
}

QByteArray CopilotApi::assembleCommentBody(const QString &prompt, const QString &lang, const QString &locale, const QString &apikey, const QString &apisecret)
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

QByteArray CopilotApi::assembleTranslateBody(const QString &prompt, const QString &src_lang, const QString &dst_lang, const QString &apikey, const QString &apisecret)
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

QByteArray CopilotApi::assembleBugfixBody(const QString &prompt,
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

void CopilotApi::processResponse(QNetworkReply *reply)
{
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qInfo() << "Error:" << reply->errorString();
        } else {
            QString replyMsg = QString::fromUtf8(reply->readAll());
            qInfo() << "Response:" << replyMsg;

            QJsonParseError error;
            QJsonDocument jsonDocument = QJsonDocument::fromJson(replyMsg.toUtf8(), &error);
            if (error.error != QJsonParseError::NoError) {
                qCritical() << "JSON parse error: " << error.errorString();
                return;
            }

            QJsonObject jsonObject = jsonDocument.object();
            QString app = jsonObject.value("result").toObject().value("app").toString();
            QJsonArray codeArray = jsonObject.value("result").toObject().value("output").toObject().value("code").toArray();
            QString code = codeArray.first().toString();

            emit response(kResponseTypes.value(app), code);
        }
    });
}
} // end namespace
