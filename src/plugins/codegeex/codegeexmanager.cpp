// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexmanager.h"
#include "common/util/custompaths.h"

#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMultiMap>
#include <QUuid>
#include <QTimer>
#include <QDateTime>

static const char *kUrlSSEChat = "https://codegeex.cn/prod/code/chatGlmSse/chat";
static const char *kUrlNewSession = "https://codegeex.cn/prod/code/chatGlmTalk/insert";
static const char *kUrlDeleteSession = "https://codegeex.cn/prod/code/chatGlmTalk/delete";

using namespace CodeGeeX;

CodeGeeXManager *CodeGeeXManager::instance()
{
    static CodeGeeXManager ins;
    return &ins;
}

void CodeGeeXManager::login()
{
    if (sessionId.isEmpty() || userId.isEmpty()) {
        sessionId = uuid();
        userId = uuid();
        saveConfig(sessionId, userId);
    }

    QString machineId = QSysInfo::machineUniqueId();
    askApi.sendLoginRequest(sessionId, machineId, userId);

    queryLoginState();
}

void CodeGeeXManager::saveConfig(const QString &sessionId, const QString &userId)
{
    QJsonObject config;
    config["sessionId"] = sessionId;
    config["userId"] = userId;

    QJsonDocument document(config);

    QFile file(configFilePath());
    file.open(QIODevice::WriteOnly);
    file.write(document.toJson());
    file.close();
}

void CodeGeeXManager::loadConfig()
{
    QFile file(configFilePath());
    if (!file.exists())
        return;

    file.open(QIODevice::ReadOnly);
    QString data = QString::fromUtf8(file.readAll());
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject config = document.object();
    if (!config.empty()) {
        sessionId = config["sessionId"].toString();
        userId = config["userId"].toString();
    }
}

void CodeGeeXManager::createNewSession()
{
    QString currentMSecsStr = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString sessionTitle("Session_" + currentMSecsStr);
    QString taskId(uuid());
    askApi.postNewSession(kUrlNewSession, sessionId, sessionTitle, taskId);
}

void CodeGeeXManager::deleteCurrentSession()
{
    if (currentTalkID.isEmpty())
        return;

    askApi.deleteSessions(kUrlDeleteSession, sessionId, { currentTalkID });
    createNewSession();
}

void CodeGeeXManager::sendMessage(const QString &prompt)
{
    QString askId = "User" + QString::number(QDateTime::currentMSecsSinceEpoch());
    MessageData msgData(askId, MessageData::Ask);
    msgData.updateData(prompt);
    Q_EMIT requestMessageUpdate(msgData);

    QMultiMap<QString, QString> history {};
    for (auto msgData : curSessionMsg) {
        history.insert(msgData.messageID(), msgData.messageData());
    }
    QString machineId = QSysInfo::machineUniqueId();
    askApi.postSSEChat(kUrlSSEChat, sessionId, prompt, machineId, history);
}

void CodeGeeXManager::onSessionCreated(const QString &talkId, bool isSuccessful)
{
    if (isSuccessful) {
        currentTalkID = talkId;
        Q_EMIT createdNewSession();
    } else {
        qWarning() << "Create session failed!";
    }
}

void CodeGeeXManager::onResponse(const QString &msgID, const QString &data, const QString &event)
{
    if (msgID.isEmpty())
        return;

//    qInfo() << "resp msg:" << msgID << data << event;
    if (!curSessionMsg.contains(msgID))
        curSessionMsg.insert(msgID, MessageData(msgID, MessageData::Anwser));

    if (!data.isEmpty()) {
        curSessionMsg[msgID].updateData(data);
        Q_EMIT requestMessageUpdate(curSessionMsg[msgID]);
    }

    if (event == "finish") {
        emit chatFinished();
    } else if (event == "add"){

    }
}

void CodeGeeXManager::recevieLoginState(AskApi::LoginState loginState)
{
    if (loginState == AskApi::LoginState::kLoginFailed) {
        qWarning() << "CodeGeeX login failed!";
        // switch to login ui.
    } else if (loginState == AskApi::LoginState::kLoginSuccess) {
        Q_EMIT loginSuccessed();
        // switch to ask page.
        queryTimer->stop();
        queryTimer->deleteLater();
    }
}

CodeGeeXManager::CodeGeeXManager(QObject *parent)
    : QObject(parent)
{
    initConnections();
    loadConfig();
    queryLoginState();
}

void CodeGeeXManager::initConnections()
{
    connect(&askApi, &AskApi::response, this, &CodeGeeXManager::onResponse);
    connect(&askApi, &AskApi::loginState, this, &CodeGeeXManager::recevieLoginState);
    connect(&askApi, &AskApi::sessionCreated, this, &CodeGeeXManager::onSessionCreated);
}

void CodeGeeXManager::queryLoginState()
{
    if (!queryTimer) {
        queryTimer = new QTimer(this);
        connect(queryTimer, &QTimer::timeout, this, [ = ] {
            if (!sessionId.isEmpty())
                askApi.sendQueryRequest(sessionId);
        });
    }

    queryTimer->start(1000);
}

void CodeGeeXManager::cleanHistoryMessage()
{
    curSessionMsg.clear();
}

QString CodeGeeXManager::configFilePath() const
{
    return CustomPaths::user(CustomPaths::Configures) + "/codegeexcfg.json";
}

QString CodeGeeXManager::uuid()
{
    QUuid uuid = QUuid::createUuid();
    return uuid.toString().replace("{", "").replace("}", "").replace("-", "");;
}
