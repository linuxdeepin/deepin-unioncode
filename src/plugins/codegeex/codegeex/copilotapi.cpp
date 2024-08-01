// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "copilotapi.h"
#include "src/common/supportfile/language.h"
#include "src/services/editor/editorservice.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QString>
#include <QHash>

using namespace dpfservice;

namespace CodeGeeX {
CopilotApi::CopilotApi(QObject *parent)
    : QObject(parent), manager(new QNetworkAccessManager(this))
{
}

void CopilotApi::postGenerate(const QString &url, const QString &code, const QString &suffix)
{
    QByteArray body = assembleGenerateBody(code, suffix);
    QNetworkReply *reply = postMessage(url, CodeGeeXManager::instance()->getSessionId(), body);
    reply->setProperty("responseType", CopilotApi::inline_completions);
    processResponse(reply);
}

void CopilotApi::postTranslate(const QString &url,
                               const QString &code,
                               const QString &dst_lang,
                               const QString &locale)
{
    QByteArray body = assembleTranslateBody(code, dst_lang, locale);
    QNetworkReply *reply = postMessage(url, CodeGeeXManager::instance()->getSessionId(), body);
    reply->setProperty("responseType", CopilotApi::multilingual_code_translate);
    processResponse(reply);
}

void CopilotApi::postComment(const QString &url,
                             const QString &code,
                             const QString &locale)
{
    QByteArray body = assembleCommandBody(code, locale, "comment");
    QNetworkReply *reply = postMessage(url, CodeGeeXManager::instance()->getSessionId(), body);
    reply->setProperty("responseType", CopilotApi::multilingual_code_comment);

    processResponse(reply);
}

void CopilotApi::postCommand(const QString &url,
                             const QString &code,
                             const QString &locale,
                             const QString &command)
{
    if (CodeGeeXManager::instance()->checkRunningState(true))
        return;
    QByteArray body = assembleCommandBody(code, locale, command);
    QNetworkReply *reply = postMessage(url, CodeGeeXManager::instance()->getSessionId(), body);
    reply->setProperty("responseType", CopilotApi::receiving_by_stream);

    emit messageSended();
    processResponse(reply);
}

QNetworkReply *CopilotApi::postMessage(const QString &url,
                                       const QString &token,
                                       const QByteArray &body)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("code-token", token.toUtf8());

    return manager->post(request, body);
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
QByteArray CopilotApi::assembleGenerateBody(const QString &prefix, const QString &suffix)
{
    auto file = getCurrentFileInfo();

    QJsonObject activeDocument;
    activeDocument.insert("path", file.first);
    activeDocument.insert("prefix", prefix);
    activeDocument.insert("suffix", suffix);
    activeDocument.insert("lang", file.second);

    QJsonObject contextItem;
    contextItem.insert("kind", "active_document");
    contextItem.insert("active_document", activeDocument);

    QJsonArray context;
    context.append(contextItem);

    QJsonObject json;
    json.insert("context", context);
    json.insert("model", completionModel);
    json.insert("lang", file.second);
    json.insert("max_new_tokens", 128);

    QJsonDocument doc(json);
    return doc.toJson();
}

QByteArray CopilotApi::assembleTranslateBody(const QString &code, const QString &dst_lang, const QString &locale)
{
    QJsonObject json;
    json.insert("lang", dst_lang);
    json.insert("code", code);
    json.insert("command", "translation");
    json.insert("locale", locale);
    json.insert("model", chatModel);

    QJsonDocument doc(json);
    return doc.toJson();
}

QByteArray CopilotApi::assembleCommandBody(const QString &code, const QString &locale, const QString &command)
{
    QJsonObject json;
    json.insert("command", command);
    json.insert("code", code);
    json.insert("talkId", CodeGeeXManager::instance()->getTalkId());
    json.insert("locale", locale);
    json.insert("model", chatModel);

    QJsonDocument doc(json);
    return doc.toJson();
}

void CopilotApi::processResponse(QNetworkReply *reply)
{
    if (reply->property("responseType") == CopilotApi::receiving_by_stream) {
        connect(CodeGeeXManager::instance(), &CodeGeeXManager::requestStop, this, [=](){
            reply->close();
        });
        connect(reply, &QNetworkReply::readyRead, this, [=]() {
            slotReadReplyStream(reply);
        });
    } else {
        connect(reply, &QNetworkReply::finished, this, [=](){
            slotReadReply(reply);
        });
    }
}

void CopilotApi::slotReadReply(QNetworkReply *reply)
{
    if (reply->error()) {
        qCritical() << "Error:" << reply->errorString();
    } else {
        QString replyMsg = QString::fromUtf8(reply->readAll());
        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(replyMsg.toUtf8(), &error);

        if (error.error != QJsonParseError::NoError) {
            qCritical() << "JSON parse error: " << error.errorString();
            return;
        }

        QJsonObject jsonObject = jsonDocument.object();
        QString code {};

        auto type = reply->property("responseType").value<CopilotApi::ResponseType>();
        if (type == CopilotApi::inline_completions) {
            auto content = jsonObject.value("inline_completions").toArray().at(0).toObject();
            code = content.value("text").toString();
            // Cut the first code segment
            auto codeLines = code.split('\n');
            code = codeLines.mid(0, codeLines.indexOf("")).join('\n');
            emit response(CopilotApi::inline_completions, code, "");
        } else if (type == CopilotApi::multilingual_code_translate) {
            auto codeLines = jsonObject.value("text").toString().split('\n');
            QString lang = codeLines.first();

            if (lang.startsWith("```"))
                lang = lang.mid(3);
            codeLines.removeFirst();
            while (codeLines.last() == "")
                codeLines.removeLast();
            if (codeLines.last() == "```")
                codeLines.removeLast();   //remove ```lang ```

            code = codeLines.join('\n');
            emit response(CopilotApi::multilingual_code_translate, code, lang);
        } else if (type == CopilotApi::multilingual_code_comment) {
            code = jsonObject.value("text").toString();
            emit response(CopilotApi::multilingual_code_comment, code, "");
        }
    }
}

void CopilotApi::slotReadReplyStream(QNetworkReply *reply)
{
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
            } else if (key == "data") {
                QJsonParseError error;
                QJsonDocument jsonDocument = QJsonDocument::fromJson(value.toUtf8(), &error);

                if (error.error != QJsonParseError::NoError) {
                    qCritical() << "JSON parse error: " << error.errorString();
                    return;
                }

                QJsonObject jsonObject = jsonDocument.object();
                data = jsonObject.value("text").toString();

                emit responseByStream(id, data, event);
            }
        }
    }
}

void CopilotApi::setModel(languageModel model)
{
    if (model == CodeGeeX::Lite) {
        chatModel = chatModelLite;
        completionModel = completionModelLite;
    } else if (model == CodeGeeX::Pro) {
        chatModel = chatModelPro;
        completionModel = completionModelPro;
    }
}

QPair<QString, QString> CopilotApi::getCurrentFileInfo()
{
    auto &ctx = dpfInstance.serviceContext();
    EditorService *editorService = ctx.service<EditorService>(EditorService::name());
    auto filePath = editorService->currentFile();
    QString fileName;
    if (QFileInfo(filePath).exists())
        fileName = QFileInfo(filePath).fileName();
    else
        fileName = "main.cpp";
    auto fileType = support_file::Language::id(filePath);
    auto fileLang = support_file::Language::idAlias(fileType);

    return qMakePair(fileName, fileLang);
}

}   // end namespace
