// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "askapi.h"
#include "codegeexmanager.h"
#include "src/common/supportfile/language.h"
#include "services/project/projectservice.h"
#include "services/window/windowservice.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QtConcurrent>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>

namespace CodeGeeX {

static int kCode_Success = 200;

AskApi::AskApi(QObject *parent)
    : QObject(parent),
      manager(new QNetworkAccessManager(this))
{
    connect(this, &AskApi::syncSendMessage, this, &AskApi::slotSendMessage);
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

void AskApi::slotSendMessage(const QString url, const QString &token, const QByteArray &body)
{
    QNetworkReply *reply = postMessage(url, token, body);
    connect(this, &AskApi::stopReceive, reply, [reply]() {
        reply->close();
    });
    processResponse(reply);
}

void AskApi::postSSEChat(const QString &url,
                         const QString &token,
                         const QString &prompt,
                         const QString &machineId,
                         const QMultiMap<QString, QString> &history,
                         const QString &talkId)
{
    QJsonArray jsonArray = convertHistoryToJSONArray(history);

#ifdef SUPPORTMINIFORGE
    auto impl = CodeGeeXManager::instance();
    impl->checkCondaInstalled();
    if (impl->isReferenceCodebase() && !impl->condaHasInstalled()) { // if not x86 or arm. @codebase can not be use
        QStringList actions { "ai_rag_install", tr("Install") };
        dpfservice::WindowService *windowService = dpfGetService(dpfservice::WindowService);
        windowService->notify(0, "AI", CodeGeeXManager::tr("The file indexing feature is not available, which may cause functions such as @codebase to not work properly."
                                          "Please install the required environment.\n the installation process may take several minutes."),
                              actions);
    }
#endif

    QtConcurrent::run([prompt, machineId, jsonArray, talkId, url, token, this]() {
        QByteArray body = assembleSSEChatBody(prompt, machineId, jsonArray, talkId);
        if (!body.isEmpty())
            emit syncSendMessage(url, token, body);
    });
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

void AskApi::setLocale(const QString &locale)
{
    this->locale = locale;
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
                    auto entry = processJsonObject(event, &jsonObject);
                    if (entry.type == "crawl")
                        emit crawledWebsite(id, entry.websites);
                    else
                        emit response(id, entry.text, event);
                }
            }
        }
    });
}

Entry AskApi::processJsonObject(const QString &event, QJsonObject *obj)
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
            for (auto it = crawlObj.begin(); it != crawlObj.end(); ++it) {
                websiteReference website;
                QString citationKey = it.key();
                QJsonObject citationObj = it.value().toObject();

                website.citation = citationKey;
                website.status = citationObj["status"].toString();
                website.url = citationObj["url"].toString();
                website.title = citationObj["title"].toString();

                entry.websites.append(website);
            }
        }
        return entry;
    }

    if (event == "finish") {
        entry.text = obj->value("text").toString();
        entry.type = event;
    }

    return entry;
}

QByteArray AskApi::assembleSSEChatBody(const QString &prompt,
                                       const QString &machineId,
                                       const QJsonArray &history,
                                       const QString &talkId)
{
    QJsonObject jsonObject;

    jsonObject.insert("prompt", prompt);
    jsonObject.insert("machineId", machineId);
    //jsonObject.insert("client", "deepin-unioncode");
    jsonObject.insert("history", history);
    jsonObject.insert("locale", locale);
    jsonObject.insert("model", model);

    using dpfservice::ProjectService;
    ProjectService *prjService = dpfGetService(ProjectService);
    auto currentProjectPath = prjService->getActiveProjectInfo().workspaceFolder();

    if (CodeGeeXManager::instance()->isReferenceCodebase() && currentProjectPath != "") {
        QJsonObject result = CodeGeeXManager::instance()->query(currentProjectPath, prompt, 20);
        QJsonArray chunks = result["Chunks"].toArray();
        if (!chunks.isEmpty()) {
            CodeGeeXManager::instance()->cleanHistoryMessage(); // incase history is too big
            if (result["Completed"].toBool() == false)
                CodeGeeXManager::instance()->notify(0, CodeGeeXManager::tr("The indexing of project %1 has not been completed, which may cause the results to be inaccurate.").arg(currentProjectPath));
            jsonObject["history"] = QJsonArray();
            QString context;
            context += prompt;
            context += "\n 参考下面这些代码片段，回答上面的问题。不要参考其他的代码和上下文，数据不够充分的情况下提示用户\n";
            for (auto chunk : chunks) {
                context += chunk.toObject()["fileName"].toString();
                context += '\n';
                context += chunk.toObject()["content"].toString();
                context += "\n\n";
            }
            jsonObject["prompt"] = context;
        } else if (CodeGeeXManager::instance()->condaHasInstalled()) {
            emit noChunksFounded();
            return {};
        }
    }

    if (!CodeGeeXManager::instance()->getReferenceFiles().isEmpty()) {
        auto fileDatas = parseFile(CodeGeeXManager::instance()->getReferenceFiles());
        jsonObject.insert("command", "file_augment");
        QJsonObject files;
        files["files"] = fileDatas;
        jsonObject.insert("files", files);
    } else if (CodeGeeXManager::instance()->isConnectToNetWork())
        jsonObject.insert("command", "online_search");

    if (!talkId.isEmpty())
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

QJsonArray AskApi::parseFile(QStringList files)
{
    QJsonArray result;

    for (auto file : files) {
        QJsonObject obj;
        obj["name"] = QFileInfo(file).fileName();
        obj["language"] = support_file::Language::id(file);
        QFile content(file);
        if (content.open(QIODevice::ReadOnly)) {
            obj["content"] = QString(content.readAll());
        }
        result.append(obj);
    }

    return result;
}

}   // end namespace
