// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include "data/messagedata.h"
#include "base/ai/abstractllm.h"
#include "services/ai/aiservice.h"

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QMutex>
#include <QIODevice>
#include <QJsonObject>

struct websiteReference
{
    QString citation;
    QString url;
    QString title;
    QString status;
};

struct RecordData
{
    QString talkId;
    QString promot;
    QString date;
};

namespace Chat {
#if defined(__x86_64__)// || defined(__aarch64__)
#define SUPPORTMINIFORGE
#endif

enum Locale {
    Zh,
    En
};
}
Q_DECLARE_METATYPE(Chat::Locale)

class ChatManager : public QObject
{
    Q_OBJECT
public:
    static ChatManager *instance();

    void checkCondaInstalled();
    bool condaHasInstalled();

    AbstractLLM *getCurrentLLM() const;
    LLMInfo getCurrentLLMInfo() const;

    void setLocale(Chat::Locale locale);

    void deleteCurrentSession();
    void deleteSession(const QString &talkId);

    void setMessage(const QString &prompt);
    void sendMessage(const QString &prompt);
    void requestAsync(const QString &prompt);
    QString requestSync(const QString &prompt); // Sync use another llm to get response. not chat

    void fetchSessionRecords();
    void fetchMessageList(const QString &talkId);
    void startReceiving();
    void stopReceiving();
    bool checkRunningState(bool state);

    QList<RecordData> sessionRecords() const;

    void connectToNetWork(bool connecting);
    bool isConnectToNetWork() const;
    QStringList getReferenceFiles() const;
    void setReferenceCodebase(bool on);
    bool isReferenceCodebase() const;
    void setReferenceFiles(const QStringList &files);
    QStringList getReferenceFiles();

    // Rag
    QString condaRootPath() const;
    void showIndexingWidget();
    Q_INVOKABLE void installConda();
    Q_INVOKABLE void generateRag(const QString &projectPath);
    QJsonObject getCurrentChunks() const { return currentChunks; }
    /*
     JsonObject:
        Query: str
        Chunks: Arr[fileName:str, content:str, similarity:float]
        Instructions: obj{name:str, description:str, content:str}
     */
    QJsonObject query(const QString &projectPath, const QString &query, int topItems);

    QString promptPreProcessing(const QString &originText);
    QString getChunks(const QString &queryText);

    // repair
    void repairDiagnostic(const QString &info);

Q_SIGNALS:
    void sendSyncRequest(const QJsonObject &obj);
    void requestMessageUpdate(const MessageData &msg);
    void chatStarted();
    void crawledWebsite(const QString &msgID, const QList<websiteReference> &websites);
    void chatFinished();
    void sessionRecordsUpdated();
    void setTextToSend(const QString &prompt);
    void requestStop();
    void notify(int type, const QString &message, QStringList actions = {});
    void showCustomWidget(QWidget *widget);
    void generateDone(const QString &path, bool failed);
    void noChunksFounded();
    void quit();
    void llmChanged(const LLMInfo &info);
    void terminated();

public Q_SLOTS:
    void slotSendSyncRequest(const QJsonObject &obj);
    void onResponse(const QString &msgID, const QString &data, AbstractLLM::ResponseState state);
    void recevieDeleteResult(const QStringList &talkIds, bool success);
    void onLLMChanged(const LLMInfo &llmInfo);

private:
    explicit ChatManager(QObject *parent = nullptr);

    void initConnections();
    void initLLM(AbstractLLM *llm);
    QString modifiedData(const QString &data);
    QString readContext(const QString &path, int codeLine);

    QString sessionId;
    QString userId;
    QString currentTalkID;
    QString responseData;

    bool codebaseEnabled = false;
    bool networkEnabled = false;
    QStringList referenceFiles;

    QMap<QString, MessageData> curSessionMsg;
    QList<RecordData> sessionRecordList {};

    bool isRunning { false };
    bool condaInstalled { false };
    QTimer installCondaTimer;
    QMutex mutex;
    QStringList indexingProject {};
    QJsonObject currentChunks;

    int answerFlag = 0; // use to identify every single answer
    AbstractLLM *chatLLM { nullptr };
    LLMInfo currentInfo;
    AbstractLLM *liteLLM { nullptr }; //Codegeex Lite.
    Chat::Locale locale { Chat::Zh };
};

#endif   // CHATMANAGER_H
