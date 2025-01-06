// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "copilotapi.h"
#include "src/common/supportfile/language.h"
#include "src/services/editor/editorservice.h"
#include "src/services/project/projectservice.h"
#include "services/window/windowservice.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <QString>
#include <QHash>

using namespace dpfservice;

void sendNotify(uint type, const QString &name, const QString &msg)
{
    auto windowSrv = dpfGetService(WindowService);
    windowSrv->notify(type, name, msg, {});
}

namespace CodeGeeX {
CopilotApi::CopilotApi(QObject *parent)
    : QObject(parent), manager(new QNetworkAccessManager(this))
{
    connect(this, &CopilotApi::asyncGenerateMessages, this, &CopilotApi::slotPostGenerateMessage);
}

void CopilotApi::slotPostGenerateMessage(const QString &url, const QByteArray &body)
{
    QNetworkReply *reply = postMessage(url, CodeGeeXManager::instance()->getSessionId(), body);
    reply->setProperty("responseType", CopilotApi::inline_completions);
    completionReply = reply;
    processResponse(reply);
}

void CopilotApi::postGenerate(const QString &url, const QString &prefix, const QString &suffix, GenerateType type)
{
    if (completionReply)
        completionReply->close();
    QtConcurrent::run([prefix, suffix, type, url, this]() {
        QByteArray body = assembleGenerateBody(prefix, suffix, type);
        emit asyncGenerateMessages(url, body);
    });
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

void CopilotApi::postInlineChat(const QString &url,
                                const QString &prompt,
                                const InlineChatInfo &info,
                                const QString &locale)
{
    QByteArray body = assembleInlineChatBody(prompt, info, locale);
    QNetworkReply *reply = postMessage(url, CodeGeeXManager::instance()->getSessionId(), body);
    reply->setProperty("responseType", CopilotApi::inline_chat);

    processResponse(reply);
}

void CopilotApi::postCommit(const QString &url,
                            const CommitMessage &message,
                            const QString &locale)
{
    QByteArray body = assembleCommitBody(message, locale);
    QNetworkReply *reply = postMessage(url, CodeGeeXManager::instance()->getSessionId(), body);
    reply->setProperty("responseType", CopilotApi::receiving_by_stream);

    processResponse(reply);
}

void CopilotApi::postCommand(const QString &url,
                             const QString &code,
                             const QString &locale,
                             const QString &command)
{
    QByteArray body = assembleCommandBody(code, locale, command);
    QNetworkReply *reply = postMessage(url, CodeGeeXManager::instance()->getSessionId(), body);
    reply->setProperty("responseType", CopilotApi::receiving_by_stream);

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
QByteArray CopilotApi::assembleGenerateBody(const QString &prefix, const QString &suffix, GenerateType type)
{
    auto file = getCurrentFileInfo();

    QJsonObject activeDocument;
    activeDocument.insert("path", file.first);
    activeDocument.insert("prefix", prefix);
    activeDocument.insert("suffix", suffix);
    activeDocument.insert("lang", file.second);

    QJsonObject activeContextItem;
    activeContextItem.insert("kind", "active_document");
    activeContextItem.insert("active_document", activeDocument);

    ProjectService *prjSrv = dpfGetService(ProjectService);
    QJsonArray context;
    context.append(activeContextItem);
    QJsonObject queryResults = CodeGeeXManager::instance()->query(prjSrv->getActiveProjectInfo().workspaceFolder(), prefix, 5);
    QJsonArray chunks = queryResults["Chunks"].toArray();

    for (auto chunk : chunks) {
        QJsonObject document;
        document.insert("path", chunk.toObject()["fileName"].toString());
        document.insert("text", chunk.toObject()["content"].toString());
        document.insert("lang", file.second);

        QJsonObject contextItem;
        contextItem.insert("kind", "document");
        contextItem.insert("document", document);
        context.append(contextItem);
    }

    QJsonObject json;
    json.insert("ide", qApp->applicationName());
    json.insert("ide_version", version());
    json.insert("context", context);
    json.insert("model", completionModel);
    json.insert("lang", file.second);
    if (type == GenerateType::Line)
        json.insert("max_new_tokens", 64);
    else
        json.insert("max_new_tokens", 128);

    QJsonDocument doc(json);
    return doc.toJson();
}

QByteArray CopilotApi::assembleInlineChatBody(const QString &prompt, const InlineChatInfo &info, const QString &locale)
{
    auto file = getCurrentFileInfo();

    QJsonObject json;
    json.insert("ide", qApp->applicationName());
    json.insert("ide_version", version());
    json.insert("lang", file.second);
    json.insert("code", info.selectedCode);
    json.insert("command", "inline_chat");
    json.insert("locale", locale);
    json.insert("talkId", CodeGeeXManager::instance()->getTalkId());
    json.insert("model", chatModel);
    QString promptWithType = prompt;
    if (info.commandType == InlineChatInfo::Chat)
        promptWithType.append(".use Chat type to answer me");
    else if (info.commandType == InlineChatInfo::Programing)
        promptWithType.append(".use Programing type to answer me");
    json.insert("prompt", promptWithType);

    QJsonObject inline_chat_obj;
    inline_chat_obj.insert("is_ast", info.is_ast);
    inline_chat_obj.insert("file_name", info.fileName);
    inline_chat_obj.insert("package_code", info.package_code);
    inline_chat_obj.insert("class_function", info.class_function);
    inline_chat_obj.insert("context_code", info.contextCode);

    json.insert("inline_chat", inline_chat_obj);
    QJsonDocument doc(json);
    return doc.toJson();
}

QByteArray CopilotApi::assembleCommitBody(const CommitMessage &message, const QString &locale)
{
    QJsonObject json;
    json.insert("ide", qApp->applicationName());
    json.insert("ide_version", version());
    json.insert("command", "commit_message_v1");
    json.insert("talkId", CodeGeeXManager::instance()->getTalkId());
    json.insert("locale", locale);
    json.insert("model", chatModel);

    QJsonObject commitObj;
    commitObj.insert("git_diff", message.git_diff);
    commitObj.insert("commit_history", message.commit_history);
    commitObj.insert("commit_type", message.commit_type);

    json.insert("commit_message", commitObj);

    QJsonDocument doc(json);
    return doc.toJson();
}

QByteArray CopilotApi::assembleCommandBody(const QString &code, const QString &locale, const QString &command)
{
    QJsonObject json;
    json.insert("ide", qApp->applicationName());
    json.insert("ide_version", version());
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
    connect(this, &CopilotApi::requestStop, this, [=]() { reply->close(); });
    if (reply->property("responseType") == CopilotApi::receiving_by_stream) {
        connect(reply, &QNetworkReply::readyRead, this, [=]() {
            slotReadReplyStream(reply);
        });
    } else {
        connect(reply, &QNetworkReply::finished, this, [=]() {
            slotReadReply(reply);
        });
    }
}

void CopilotApi::slotReadReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Error:" << reply->errorString() << reply->error();
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            auto type = reply->property("responseType").value<CopilotApi::ResponseType>();
            sendNotify(2, "AI" ,reply->errorString());
            emit response(type, "", "");
        }
    } else {
        QString replyMsg = QString::fromUtf8(reply->readAll());
        QJsonParseError error;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(replyMsg.toUtf8(), &error);
        auto type = reply->property("responseType").value<CopilotApi::ResponseType>();
        if (error.error != QJsonParseError::NoError) {
            qCritical() << "JSON parse error: " << error.errorString();
            emit response(type, "", "");
            return;
        }

        QJsonObject jsonObject = jsonDocument.object();
        QString code {};
        if (type == CopilotApi::inline_completions) {
            auto content = jsonObject.value("inline_completions").toArray().at(0).toObject();
            code = content.value("text").toString();
            if (content.value("finish_reason").toString() == "length") {
                // Due to the length limit of the code, the last line will be discarded when the code is truncated.
                auto codeLines = code.split('\n');
                if (codeLines.size() > 1)
                    codeLines.removeLast();
                code = codeLines.join('\n');
            }

            completionReply = nullptr;

            // all '\n'
            if (code.split('\n', QString::SkipEmptyParts).isEmpty())
                return;
            emit response(CopilotApi::inline_completions, code, "");
        } else if (type == CopilotApi::multilingual_code_comment) {
            code = jsonObject.value("text").toString();
            emit response(CopilotApi::multilingual_code_comment, code, "");
        } else if (type == CopilotApi::inline_chat) {
            code = jsonObject.value("text").toString();
            emit response(CopilotApi::inline_chat, code, "");
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

languageModel CopilotApi::model() const
{
    if (chatModel == chatModelLite)
        return Lite;
    return Pro;
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

    // The above LANGUAGE class supports fewer file languages, and unknown file languages are temporarily represented by suffix.
    if (fileLang.isEmpty())
        fileLang = QFileInfo(filePath).suffix();
    return qMakePair(fileName, fileLang);
}

}   // end namespace
