// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexmanager.h"
#include "copilot.h"
#include "common/util/custompaths.h"

#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMultiMap>
#include <QUuid>
#include <QTimer>
#include <QDateTime>

static const char *kUrlSSEChat = "https://api.codegeex.cn:8443/tx/v3/chat/message";
//static const char *kUrlSSEChat = "https://codegeex.cn/prod/code/chatGlmSse/chat";
static const char *kUrlNewSession = "https://codegeex.cn/prod/code/chatGlmTalk/insert";
static const char *kUrlDeleteSession = "https://codegeex.cn/prod/code/chatGlmTalk/delete";
static const char *kUrlQuerySession = "https://codegeex.cn/prod/code/chatGlmTalk/selectList";
static const char *kUrlQueryMessage = "https://codegeex.cn/prod/code/chatGmlMsg/selectList";

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

bool CodeGeeXManager::isLoggedIn() const
{
    return isLogin;
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

void CodeGeeXManager::setLocale(CodeGeeX::locale locale)
{
    if (locale == CodeGeeX::Zh) {
        askApi.setLocale("zh");
        Copilot::instance()->setLocale("zh");
    } else if (locale == CodeGeeX::En) {
        askApi.setLocale("en");
        Copilot::instance()->setLocale("en");
    }
}

void CodeGeeXManager::setCurrentModel(languageModel model)
{
    Copilot::instance()->setCurrentModel(model);
    if (model == Lite)
        askApi.setModel(chatModelLite);
    else if (model == Pro)
        askApi.setModel(chatModelPro);
}

void CodeGeeXManager::connectToNetWork(bool connecting)
{
    isConnecting = connecting;
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

void CodeGeeXManager::deleteSession(const QString &talkId)
{
    askApi.deleteSessions(kUrlDeleteSession, sessionId, { talkId });
}

void CodeGeeXManager::setMessage(const QString &prompt)
{
    Q_EMIT setTextToSend(prompt);
}

void CodeGeeXManager::sendMessage(const QString &prompt)
{
    QString askId = "User" + QString::number(QDateTime::currentMSecsSinceEpoch());
    MessageData msgData(askId, MessageData::Ask);
    msgData.updateData(prompt);
    Q_EMIT requestMessageUpdate(msgData);

    if (currentChat.first.isEmpty())
        currentChat.first = prompt;
    QMultiMap<QString, QString> history {};
    for (auto chat : chatHistory) {
        history.insert(chat.first, chat.second);
    }
    QString machineId = QSysInfo::machineUniqueId();
    QString talkId = currentTalkID;
    askApi.postSSEChat(kUrlSSEChat, sessionId, prompt, machineId, history, talkId);

    startReceiving();
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

    auto msgData = modifiedData(data);
    if (event == "finish") {
        responseData.clear();
        if (!currentChat.first.isEmpty() && currentChat.second.isEmpty()) {
            currentChat.second = msgData;
            chatHistory.append(currentChat);
            chatRecord empty {};
            currentChat.swap(empty);
        }
        isRunning = false;
        emit chatFinished();
        return;
    } else if (event == "add") {
        responseData += msgData;
        if (!curSessionMsg.contains(msgID))
            curSessionMsg.insert(msgID, MessageData(msgID, MessageData::Anwser));

        if (!data.isEmpty()) {
            curSessionMsg[msgID].updateData(responseData);
            Q_EMIT requestMessageUpdate(curSessionMsg[msgID]);
        }
    }
}

void CodeGeeXManager::recevieLoginState(AskApi::LoginState loginState)
{
    if (loginState == AskApi::LoginState::kLoginFailed) {
        //qWarning() << "CodeGeeX login failed!";
        // switch to login ui.
    } else if (loginState == AskApi::LoginState::kLoginSuccess) {
        isLogin = true;
        Q_EMIT loginSuccessed();
        // switch to ask page.
        if (queryTimer) {
            queryTimer->stop();
            queryTimer->deleteLater();
            queryTimer = nullptr;
        }
    } else if (loginState == AskApi::LoginState::kLoginOut) {
        isLogin = false;
        Q_EMIT logoutSuccessed();
    }
}

void CodeGeeXManager::recevieToTranslate(const QString &codeText)
{
    if (isLogin && !codeText.isEmpty())
        Q_EMIT requestToTransCode(codeText);
}

void CodeGeeXManager::recevieSessionRecords(const QVector<AskApi::SessionRecord> &records)
{
    sessionRecordList.clear();

    for (auto record : records) {
        RecordData data;
        data.talkId = record.talkId;
        data.promot = record.prompt;
        data.date = record.createdTime;
        sessionRecordList.append(data);
    }

    Q_EMIT sessionRecordsUpdated();
}

void CodeGeeXManager::showHistoryMessage(const QVector<AskApi::MessageRecord> &records)
{
    for (auto index = records.size() - 1; index >= 0; index--) {
        auto messageId = QString::number(QDateTime::currentMSecsSinceEpoch());
        auto record = records[index];
        MessageData askData(messageId + "Ask", MessageData::Ask);
        askData.updateData(record.input);
        Q_EMIT requestMessageUpdate(askData);

        //正常情况下，答案从网络回复中流式解析。这里如果一次性发送，无法解析代码，需要以流式方式发送，读的长度过长可能会导致解析"```"标志时吞掉其他字符
        MessageData ansData(messageId + "Anwser", MessageData::Anwser);
        QTextStream stream(&record.output);
        QString anwser;
        while (!stream.atEnd()) {
            anwser += stream.read(3);
            ansData.updateData(anwser);
            Q_EMIT requestMessageUpdate(ansData);
        }
    }
}

void CodeGeeXManager::recevieDeleteResult(const QStringList &talkIds, bool success)
{
    if (success) {
        for (const QString &talkId : talkIds) {
            int i = 0;
            while (i < sessionRecordList.length()) {
                if (sessionRecordList[i].talkId == talkId)
                    sessionRecordList.removeAt(i);
                else
                    ++i;
            }
        }
        Q_EMIT sessionRecordsUpdated();
    } else {
        qWarning() << "Delete history session failed!";
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
    connect(&askApi, &AskApi::crawledWebsite, this, &CodeGeeXManager::crawledWebsite);
    connect(&askApi, &AskApi::loginState, this, &CodeGeeXManager::recevieLoginState);
    connect(&askApi, &AskApi::sessionCreated, this, &CodeGeeXManager::onSessionCreated);
    connect(&askApi, &AskApi::getSessionListResult, this, &CodeGeeXManager::recevieSessionRecords);
    connect(&askApi, &AskApi::sessionDeleted, this, &CodeGeeXManager::recevieDeleteResult);
    connect(&askApi, &AskApi::getMessageListResult, this, &CodeGeeXManager::showHistoryMessage);

    connect(Copilot::instance(), &Copilot::translatingText, this, &CodeGeeXManager::recevieToTranslate);
    connect(Copilot::instance(), &Copilot::response, this, &CodeGeeXManager::onResponse);
    connect(Copilot::instance(), &Copilot::messageSended, this, &CodeGeeXManager::startReceiving);

    connect(this, &CodeGeeXManager::requestStop, &askApi, &AskApi::stopReceive);
}

void CodeGeeXManager::queryLoginState()
{
    if (!queryTimer) {
        queryTimer = new QTimer(this);
        connect(queryTimer, &QTimer::timeout, this, [=] {
            if (!sessionId.isEmpty())
                askApi.sendQueryRequest(sessionId);
        });
    }

    queryTimer->start(1000);
}

void CodeGeeXManager::logout()
{
    if (!isLogin) {
        qWarning() << "cant`t logout without login";
        return;
    }
    askApi.logout(sessionId);
}

void CodeGeeXManager::cleanHistoryMessage()
{
    chatHistory.clear();
    curSessionMsg.clear();
}

void CodeGeeXManager::fetchSessionRecords()
{
    askApi.getSessionList(kUrlQuerySession, sessionId, 1, 50);
}

void CodeGeeXManager::fetchMessageList(const QString &talkId)
{
    askApi.getMessageList(kUrlQueryMessage, sessionId, 1, 50, talkId);
}

void CodeGeeXManager::startReceiving()
{
    isRunning = true;
    emit chatStarted();
}

void CodeGeeXManager::stopReceiving()
{
    isRunning = false;
    responseData.clear();
    chatRecord empty {};
    currentChat.swap(empty);
    emit requestStop();
}

bool CodeGeeXManager::checkRunningState(bool state)
{
    return isRunning == state;
}

QList<RecordData> CodeGeeXManager::sessionRecords() const
{
    return sessionRecordList;
}

QString CodeGeeXManager::configFilePath() const
{
    return CustomPaths::user(CustomPaths::Configures) + "/codegeexcfg.json";
}

QString CodeGeeXManager::uuid()
{
    QUuid uuid = QUuid::createUuid();
    return uuid.toString().replace("{", "").replace("}", "").replace("-", "");
}

QString CodeGeeXManager::getSessionId() const
{
    return sessionId;
}

QString CodeGeeXManager::getTalkId() const
{
    return currentTalkID;
}

QString CodeGeeXManager::modifiedData(const QString &data)
{
    auto retData = data;
    retData.replace("\\n", "\n");
    retData.replace("\\\"", "\"");
    retData.replace("\\\\", "\\");

    return retData;
}
