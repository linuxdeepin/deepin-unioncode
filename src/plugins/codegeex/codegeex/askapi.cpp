// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "askapi.h"
#include "codegeexmanager.h"
#include "src/common/supportfile/language.h"
#include "services/project/projectservice.h"
#include "services/window/windowservice.h"
#include "services/editor/editorservice.h"
#include "common/type/constants.h"
#include <services/ai/aiservice.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QtConcurrent>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>

namespace CodeGeeX {
static const QString PrePrompt = "你是一个智能编程助手，可以回答用户任何的问题。问题中可能会带有相关的context，这些context来自工程相关的文件，你要结合这些上下文回答用户问题。 请注意：\n1.用户问题中以@符号开始的标记代表着context内容。/n2.按正确的语言回答，不要被上下文中的字符影响.";

static int kCode_Success = 200;

class AskApiPrivate : public QObject
{
public:
    explicit AskApiPrivate(AskApi *qq);

    QNetworkReply *postMessage(const QString &url, const QString &token, const QByteArray &body);
    QNetworkReply *getMessage(const QString &url, const QString &token);
    void processResponse(QNetworkReply *reply);
    Entry processJsonObject(const QString &event, QJsonObject *obj);

    QByteArray assembleSSEChatBody(const QString &prompt,
                                   const QString &machineId,
                                   const QJsonArray &history,
                                   const QString &talkId);

    QByteArray assembleNewSessionBody(const QString &prompt,
                                      const QString &talkId);

    QByteArray assembleDelSessionBody(const QStringList &talkIds);

    QByteArray jsonToByteArray(const QJsonObject &jsonObject);
    QJsonObject toJsonOBject(QNetworkReply *reply);
    QJsonArray parseFile(QStringList files);

public:
    AskApi *q;

    QNetworkAccessManager *manager = nullptr;
    QString model = chatModelLite;
    QString locale = "zh";
    bool codebaseEnabled = false;
    bool networkEnabled = false;
    bool terminated = false;
    QStringList referenceFiles;
};

AskApiPrivate::AskApiPrivate(AskApi *qq)
    : q(qq),
      manager(new QNetworkAccessManager(qq))
{
    connect(q, &AskApi::stopReceive, this, [=]() { terminated = true; });
}

QNetworkReply *AskApiPrivate::postMessage(const QString &url, const QString &token, const QByteArray &body)
{
    if (terminated)
        return nullptr;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("code-token", token.toUtf8());

    if (QThread::currentThread() != qApp->thread()) {
        QNetworkAccessManager* threadManager(new QNetworkAccessManager);
        AskApi::connect(QThread::currentThread(), &QThread::finished, threadManager, &QNetworkAccessManager::deleteLater);
        return threadManager->post(request, body);
    }
    return manager->post(request, body);
}

QNetworkReply *AskApiPrivate::getMessage(const QString &url, const QString &token)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("code-token", token.toUtf8());

    if (QThread::currentThread() != qApp->thread()) {
        QNetworkAccessManager* threadManager(new QNetworkAccessManager);
        AskApi::connect(QThread::currentThread(), &QThread::finished, threadManager, &QNetworkAccessManager::deleteLater);
        return threadManager->get(request);
    }
    return manager->get(request);
}

void AskApiPrivate::processResponse(QNetworkReply *reply)
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

                    QJsonObject jsonObject = jsonDocument.object();
                    auto entry = processJsonObject(event, &jsonObject);
                    if (error.error != QJsonParseError::NoError) {
                        qCritical() << "JSON parse error: " << error.errorString();
                        if (event == "finish") {
                            emit q->response(id, entry.text, event);
                            return;
                        }
                        continue;
                    }

                    if (entry.type == "crawl")
                        emit q->crawledWebsite(id, entry.websites);
                    else
                        emit q->response(id, entry.text, event);
                }
            }
        }
    });
}

Entry AskApiPrivate::processJsonObject(const QString &event, QJsonObject *obj)
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

QByteArray AskApiPrivate::assembleSSEChatBody(const QString &prompt, const QString &machineId,
                                              const QJsonArray &history, const QString &talkId)
{
    QJsonObject jsonObject;
    jsonObject.insert("ide", qApp->applicationName());
    jsonObject.insert("ide_version", version());
    jsonObject.insert("prompt", prompt);
    jsonObject.insert("machineId", machineId);
    jsonObject.insert("history", history);
    jsonObject.insert("locale", locale);
    jsonObject.insert("model", model);

    if (!referenceFiles.isEmpty()) {
        auto fileDatas = parseFile(referenceFiles);
        jsonObject.insert("command", "file_augment");
        QJsonObject files;
        files["files"] = fileDatas;
        jsonObject.insert("files", files);
    } else if (networkEnabled)
        jsonObject.insert("command", "online_search_v1");

    if (!talkId.isEmpty())
        jsonObject.insert("talkId", talkId);

    return jsonToByteArray(jsonObject);
}

QByteArray AskApiPrivate::assembleNewSessionBody(const QString &prompt, const QString &talkId)
{
    QJsonObject jsonObject;
    jsonObject.insert("prompt", prompt);
    jsonObject.insert("talkId", talkId);

    return jsonToByteArray(jsonObject);
}

QByteArray AskApiPrivate::assembleDelSessionBody(const QStringList &talkIds)
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

QByteArray AskApiPrivate::jsonToByteArray(const QJsonObject &jsonObject)
{
    QJsonDocument doc(jsonObject);
    return doc.toJson();
}

QJsonObject AskApiPrivate::toJsonOBject(QNetworkReply *reply)
{
    QString response = QString::fromUtf8(reply->readAll());
    QJsonDocument document = QJsonDocument::fromJson(response.toUtf8());
    return document.object();
}

QJsonArray AskApiPrivate::parseFile(QStringList files)
{
    QJsonArray result;
    auto editorSrv = dpfGetService(dpfservice::EditorService);

    for (auto file : files) {
        QJsonObject obj;
        obj["name"] = QFileInfo(file).fileName();
        obj["language"] = support_file::Language::id(file);

        QString fileContent = editorSrv->fileText(file);

        if (fileContent.isEmpty()) {
            QFile content(file);
            if (content.open(QIODevice::ReadOnly)) {
                obj["content"] = QString(content.read(20000));
            }
        } else {
            obj["content"] = QString(fileContent.mid(0, 20000));
        }
        result.append(obj);
    }

    return result;
}

AskApi::AskApi(QObject *parent)
    : QObject(parent),
      d(new AskApiPrivate(this))
{
    connect(this, &AskApi::syncSendMessage, this, &AskApi::slotSendMessage);
    connect(this, &AskApi::notify, this, [](int type, const QString &message) {
        using namespace dpfservice;
        WindowService *windowService = dpfGetService(WindowService);
        windowService->notify(type, "Ai", message, QStringList {});
    });
}

AskApi::~AskApi()
{
    delete d;
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

    QNetworkReply *reply = d->getMessage(url, codeToken);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = d->toJsonOBject(reply);
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

    QNetworkReply *reply = d->getMessage(url, codeToken);
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = d->toJsonOBject(reply);
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
    QNetworkReply *reply = d->postMessage(url, token, body);
    connect(this, &AskApi::stopReceive, reply, [reply]() {
        reply->abort();
    });
    d->processResponse(reply);
}

void AskApi::postSSEChat(const QString &url,
                         const QString &token,
                         const QString &prompt,
                         const QString &machineId,
                         const QMultiMap<QString, QString> &history,
                         const QString &talkId)
{
    d->terminated = false;
    QJsonArray jsonArray = convertHistoryToJSONArray(history);

#ifdef SUPPORTMINIFORGE
    auto impl = CodeGeeXManager::instance();
    impl->checkCondaInstalled();
    if (d->codebaseEnabled && !impl->condaHasInstalled()) {   // if not x86 or arm. @codebase can not be use
        QStringList actions { "ai_rag_install", tr("Install") };
        dpfservice::WindowService *windowService = dpfGetService(dpfservice::WindowService);
        windowService->notify(0, "AI", CodeGeeXManager::tr("The file indexing feature is not available, which may cause functions such as @codebase to not work properly."
                                                           "Please install the required environment.\n the installation process may take several minutes."),
                              actions);
    }
#endif

    QByteArray body = d->assembleSSEChatBody(prompt, machineId, jsonArray, talkId);
    if (!body.isEmpty())
        emit syncSendMessage(url, token, body);
}

QString AskApi::syncQuickAsk(const QString &url,
                                  const QString &token,
                                  const QString &prompt,
                                  const QString &talkId)
{
    d->terminated = false;

    QJsonObject jsonObject;
    jsonObject.insert("ide", qApp->applicationName());
    jsonObject.insert("ide_version", version());
    jsonObject.insert("prompt", prompt);
    jsonObject.insert("history", {});
    jsonObject.insert("locale", d->locale);
    jsonObject.insert("model", chatModelLite);
    jsonObject.insert("talkId", talkId);
    jsonObject.insert("stream", false);

    QByteArray body = d->jsonToByteArray(jsonObject);
    QNetworkReply *reply = d->postMessage(url, token, body);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, reply, [&]() {
        loop.exit();
    });
    connect(this, &AskApi::stopReceive, reply, [reply, &loop]() {
        reply->abort();
        loop.exit();
    });
    loop.exec();

    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &error);
    QJsonObject obj = jsonDocument.object();
    if (error.error != QJsonParseError::NoError) {
        qCritical() << "JSON parse error: " << error.errorString();
        return "";
    }

    return obj["text"].toString();
}

void AskApi::postNewSession(const QString &url,
                            const QString &token,
                            const QString &prompt,
                            const QString &talkId)
{
    d->terminated = false;
    QByteArray body = d->assembleNewSessionBody(prompt, talkId);
    QNetworkReply *reply = d->postMessage(url, token, body);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = d->toJsonOBject(reply);
        int code = jsonObject["code"].toInt();

        emit sessionCreated(talkId, code == kCode_Success);
    });
}

void AskApi::getSessionList(const QString &url, const QString &token, int pageNumber, int pageSize)
{
    QString urlWithParameter = QString(url + "?pageNum=%1&pageSize=%2").arg(pageNumber).arg(pageSize);
    QNetworkReply *reply = d->getMessage(urlWithParameter, token);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = d->toJsonOBject(reply);
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
    QNetworkReply *reply = d->getMessage(urlWithParameter, token);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = d->toJsonOBject(reply);

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
    d->terminated = false;
    QByteArray body = d->assembleDelSessionBody(talkIds);
    QNetworkReply *reply = d->postMessage(url, token, body);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error()) {
            qCritical() << "Error:" << reply->errorString();
            return;
        }
        QJsonObject jsonObject = d->toJsonOBject(reply);
        int code = jsonObject["code"].toInt();
        emit sessionDeleted(talkIds, code == kCode_Success);
    });
}

void AskApi::setModel(const QString &model)
{
    d->model = model;
}

void AskApi::setLocale(const QString &locale)
{
    d->locale = locale;
}

void AskApi::setNetworkEnabled(bool enabled)
{
    d->networkEnabled = enabled;
}

bool AskApi::networkEnabled() const
{
    return d->networkEnabled;
}

void AskApi::setReferenceFiles(const QStringList &fileList)
{
    d->referenceFiles = fileList;
}

QStringList AskApi::referenceFiles() const
{
    return d->referenceFiles;
}

void AskApi::setCodebaseEnabled(bool enabled)
{
    d->codebaseEnabled = enabled;
}

bool AskApi::codebaseEnabled() const
{
    return d->codebaseEnabled;
}

}   // end namespace
