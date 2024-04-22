// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXMANAGER_H
#define CODEGEEXMANAGER_H

#include "codegeex/askapi.h"
#include "data/messagedata.h"

#include <QObject>
#include <QMap>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

struct RecordData
{
    QString talkId;
    QString promot;
    QString date;
};

namespace CodeGeeX {
    static const char* chatModelLite = "codegeex-chat-lite";
    static const char* chatModelPro = "codegeex-chat-pro";

    static const char* completionModelLite = "codegeex-lite";
    static const char* completionModelPro = "codegeex-pro";

    enum languageModel{
        Lite,
        Pro
    };

    enum locale{
        Zh,
        En
    };
}
Q_DECLARE_METATYPE(CodeGeeX::languageModel)

typedef QPair<QString, QString> chatRecord;
class CodeGeeXManager : public QObject
{
    Q_OBJECT
public:
    static CodeGeeXManager *instance();

    Q_INVOKABLE void login();
    bool isLoggedIn() const;

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

Q_SIGNALS:
    void loginSuccessed();
    void logoutSuccessed();
    void createdNewSession();
    void requestMessageUpdate(const MessageData &msg);
    void requestToTransCode(const QString &code);
    void chatStarted();
    void chatFinished();
    void sessionRecordsUpdated();
    void setTextToSend(const QString &prompt);
    void requestStop();

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
};

#endif   // CODEGEEXMANAGER_H
