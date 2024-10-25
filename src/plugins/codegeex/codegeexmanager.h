// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXMANAGER_H
#define CODEGEEXMANAGER_H

#include "codegeex/askapi.h"
#include "data/messagedata.h"

#include <QObject>
#include <QMap>
#include <QTimer>

struct RecordData
{
    QString talkId;
    QString promot;
    QString date;
};

namespace CodeGeeX {
static const char *chatModelLite = "codegeex-4";
static const char *chatModelPro = "codegeex-chat-pro";

static const char *completionModelLite = "codegeex-lite";
static const char *completionModelPro = "codegeex-pro";

#if defined(__x86_64__)// || defined(__aarch64__)
#define SUPPORTMINIFORGE
#endif

enum languageModel {
    Lite,
    Pro
};

enum locale {
    Zh,
    En
};
}
Q_DECLARE_METATYPE(CodeGeeX::languageModel)
Q_DECLARE_METATYPE(CodeGeeX::locale)

typedef QPair<QString, QString> chatRecord;
class CodeGeeXManager : public QObject
{
    Q_OBJECT
public:
    static CodeGeeXManager *instance();

    Q_INVOKABLE void login();
    bool isLoggedIn() const;
    void checkCondaInstalled();
    bool condaHasInstalled();

    void saveConfig(const QString &sessionId, const QString &userId);
    void loadConfig();

    void setLocale(CodeGeeX::locale locale);
    void setCurrentModel(CodeGeeX::languageModel model);

    void createNewSession();
    void deleteCurrentSession();
    void deleteSession(const QString &talkId);

    void setMessage(const QString &prompt);
    void sendMessage(const QString &prompt);
    void queryLoginState();

    void cleanHistoryMessage();

    void fetchSessionRecords();
    void fetchMessageList(const QString &talkId);
    void startReceiving();
    void stopReceiving();
    bool checkRunningState(bool state);

    QString getSessionId() const;
    QString getTalkId() const;
    QList<RecordData> sessionRecords() const;

    void connectToNetWork(bool connecting);
    bool isConnectToNetWork() const;
    QStringList getReferenceFiles() const;
    void setReferenceCodebase(bool on);
    bool isReferenceCodebase() const;
    void setReferenceFiles(const QStringList &files);

    // Rag
    QString condaRootPath() const;
    void showIndexingWidget();
    Q_INVOKABLE void installConda();
    Q_INVOKABLE void generateRag(const QString &projectPath);
    /*
     JsonObject:
        Query: str
        Chunks: Arr[fileName:str, content:str, similarity:float]
        Instructions: obj{name:str, description:str, content:str}
     */
    QJsonObject query(const QString &projectPath, const QString &query, int topItems);

Q_SIGNALS:
    void loginSuccessed();
    void logoutSuccessed();
    void createdNewSession();
    void requestMessageUpdate(const MessageData &msg);
    void requestToTransCode(const QString &code);
    void chatStarted();
    void crawledWebsite(const QString &msgID, const QList<CodeGeeX::websiteReference> &websites);
    void searching(const QString &searchText);
    void terminated();
    void chatFinished();
    void sessionRecordsUpdated();
    void setTextToSend(const QString &prompt);
    void requestStop();
    void notify(int type, const QString &message);
    void showCustomWidget(QWidget *widget);
    void generateDone(const QString &path, bool failed);
    void quit();

public Q_SLOTS:
    void onSessionCreated(const QString &talkId, bool isSuccessful);
    void onResponse(const QString &msgID, const QString &data, const QString &event);
    void recevieLoginState(CodeGeeX::AskApi::LoginState loginState);
    void recevieToTranslate(const QString &codeText);
    void recevieSessionRecords(const QVector<CodeGeeX::AskApi::SessionRecord> &records);
    void recevieDeleteResult(const QStringList &talkIds, bool success);
    void showHistoryMessage(const QVector<CodeGeeX::AskApi::MessageRecord> &records);
    void logout();

private:
    explicit CodeGeeXManager(QObject *parent = nullptr);

    void initConnections();
    QString modifiedData(const QString &data);

    QString configFilePath() const;
    QString uuid();

    CodeGeeX::AskApi askApi;
    QString sessionId;
    QString userId;
    QString currentTalkID;
    QString responseData;

    QMap<QString, MessageData> curSessionMsg;
    QList<chatRecord> chatHistory {};
    chatRecord currentChat {};
    QList<RecordData> sessionRecordList {};

    QTimer *queryTimer { nullptr };
    bool isLogin { false };
    bool isRunning { false };
    bool condaInstalled { false };
    QTimer installCondaTimer;
    QStringList indexingProject {};
};

#endif   // CODEGEEXMANAGER_H
