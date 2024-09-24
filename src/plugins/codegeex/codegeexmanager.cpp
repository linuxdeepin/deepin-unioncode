// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexmanager.h"
#include "copilot.h"
#include "common/util/custompaths.h"
#include "services/window/windowservice.h"
#include "services/window/windowelement.h"
#include "services/terminal/terminalservice.h"

#include <QDebug>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMultiMap>
#include <QUuid>
#include <QTimer>
#include <QDateTime>

static const char *kUrlSSEChat = "https://codegeex.cn/prod/code/chatCodeSseV3/chat";
//static const char *kUrlSSEChat = "https://codegeex.cn/prod/code/chatGlmSse/chat";
static const char *kUrlNewSession = "https://codegeex.cn/prod/code/chatGlmTalk/insert";
static const char *kUrlDeleteSession = "https://codegeex.cn/prod/code/chatGlmTalk/delete";
static const char *kUrlQuerySession = "https://codegeex.cn/prod/code/chatGlmTalk/selectList";
static const char *kUrlQueryMessage = "https://codegeex.cn/prod/code/chatGmlMsg/selectList";

using namespace CodeGeeX;
using dpfservice::WindowService;

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

void CodeGeeXManager::checkCondaInstalled()
{
    if (condaInstalled)
        return;
    QProcess process;
    QStringList arguments;
    arguments << "env" << "list";

    process.start(condaRootPath() + "/miniforge/condabin/conda", arguments);
    process.waitForFinished();

    QString output = process.readAll();
    condaInstalled = output.contains("deepin_unioncode_env");
}

bool CodeGeeXManager::condaHasInstalled()
{
    return condaInstalled;
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

QString CodeGeeXManager::condaRootPath() const
{
    return QDir::homePath() + "/.unioncode";
}

void CodeGeeXManager::installConda()
{
    QString scriptPath = CustomPaths::CustomPaths::global(CustomPaths::Scripts) + "/rag/install.sh";
//    QProcess process;
//    process.setProgram("bash");
//    process.setArguments(QStringList() << scriptPath << condaRootPath());
//    process.startDetached();

    QMetaObject::invokeMethod(this, [=, scriptPath]() {
        auto terminalServ = dpfGetService(dpfservice::TerminalService);
        WindowService *windowService = dpfGetService(WindowService);
        windowService->switchContextWidget(dpfservice::TERMINAL_TAB_TEXT);
        terminalServ->executeCommand("install", "bash", QStringList() << scriptPath << condaRootPath(), condaRootPath(), QStringList());
    });
}

void CodeGeeXManager::generateRag(const QString &projectPath)
{
    if (indexingProject.contains(projectPath))
        return;
    bool failed = false;
    indexingProject.append(projectPath);
    QProcess process;
    QObject::connect(&process, &QProcess::readyReadStandardError, &process, [&, projectPath]() {
        if (!failed) // only notify once
            QMetaObject::invokeMethod(this, [&, projectPath](){
                WindowService *windowService = dpfGetService(WindowService);
                windowService->notify(2, "Ai", tr("The error occurred when performing rag on project %1.").arg(projectPath), QStringList{});
            });
        failed = true;
        qInfo() << "Error:" << process.readAllStandardError() << "\n";
    });
    QObject::connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, [&](int exitCode, QProcess::ExitStatus exitStatus) {
                         qInfo() << "Python script finished with exit code" << exitCode  << "Exit!!!";
                     });
    qInfo() << "start rag project:" << projectPath;
    QString ragPath = CustomPaths::CustomPaths::global(CustomPaths::Scripts) + "/rag";
    process.setWorkingDirectory(ragPath);
    auto generatePyPath = ragPath + "/generate.py";
    auto pythonPath = condaRootPath() + "/miniforge/envs/deepin_unioncode_env/bin/python";
    auto modelPath = CustomPaths::CustomPaths::global(CustomPaths::Models);
    if (!QFileInfo(pythonPath).exists())
        return;
    process.start(pythonPath, QStringList() << generatePyPath << modelPath << projectPath);
    process.waitForFinished(-1);
    indexingProject.removeOne(projectPath);
}

/*
 JsonObject:
    Query: str
    Chunks: Arr[fileName:str, content:str]
    Instructions: obj{name:str, description:str, content:str}
*/
QJsonObject CodeGeeXManager::query(const QString &projectPath, const QString &query, int topItems)
{
    QProcess process;
    QObject::connect(&process, &QProcess::readyReadStandardError, &process, [&]() {
        qInfo() << "Error:" << process.readAllStandardError() << "\n";
    });

    // If modified, update the Python file accordingly.
    auto pythonPath = condaRootPath() + "/miniforge/envs/deepin_unioncode_env/bin/python";
    if (!QFileInfo(pythonPath).exists() || !QFileInfo(condaRootPath() +"/index.sqlite").exists())
        return {};

    auto modelPath = CustomPaths::CustomPaths::global(CustomPaths::Models);
    QString ragPath = CustomPaths::CustomPaths::global(CustomPaths::Scripts) + "/rag";
    process.setWorkingDirectory(ragPath);
    auto queryPyPath = ragPath + "/query.py";
    process.start(pythonPath, QStringList() << queryPyPath << modelPath << projectPath << query << QString::number(topItems));
    process.waitForFinished();
    auto test = process.readAll();
    QJsonDocument document = QJsonDocument::fromJson(test);

    auto obj = document.object();
    return obj;
}
