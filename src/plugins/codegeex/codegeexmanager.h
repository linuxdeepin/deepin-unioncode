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

struct RecordData {
    QString talkId;
    QString promot;
    QString date;
};

class CodeGeeXManager : public QObject
{
    Q_OBJECT
public:
    static CodeGeeXManager *instance();

    void login();

    void saveConfig(const QString &sessionId, const QString &userId);
    void loadConfig();

    void createNewSession();
    void deleteCurrentSession();
    void deleteSession(const QString &talkId);

    void sendMessage(const QString &prompt);
    void queryLoginState();

    void cleanHistoryMessage();

    void fetchSessionRecords();
    QList<RecordData> sessionRecords() const;

Q_SIGNALS:
    void loginSuccessed();
    void createdNewSession();
    void requestMessageUpdate(const MessageData &msg);
    void requestToTransCode(const QString &code);
    void chatStarted();
    void chatFinished();
    void sessionRecordsUpdated();

public Q_SLOTS:
    void onSessionCreated(const QString &talkId, bool isSuccessful);
    void onResponse(const QString &msgID, const QString &data, const QString &event);
    void recevieLoginState(CodeGeeX::AskApi::LoginState loginState);
    void recevieToTranslate(const QString &codeText);
    void recevieSessionRecords(const QVector<CodeGeeX::AskApi::SessionRecord> &records);
    void recevieDeleteResult(const QStringList &talkIds, bool success);

private:
    explicit CodeGeeXManager(QObject *parent = nullptr);

    void initConnections();

    QString configFilePath() const;
    QString uuid();

    CodeGeeX::AskApi askApi;
    QString sessionId;
    QString userId;
    QString currentTalkID;

    QMap<QString, MessageData> curSessionMsg;
    QList<RecordData> sessionRecordList {};

    QTimer *queryTimer { nullptr };
    bool isLogin { false };
};

#endif // CODEGEEXMANAGER_H
