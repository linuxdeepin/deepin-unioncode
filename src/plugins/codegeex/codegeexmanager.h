// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXMANAGER_H
#define CODEGEEXMANAGER_H

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

namespace CodeGeeX {
#if defined(__x86_64__)// || defined(__aarch64__)
#define SUPPORTMINIFORGE
#endif

enum Locale {
    Zh,
    En
};
}
Q_DECLARE_METATYPE(CodeGeeX::Locale)

class CodeGeeXManager : public QObject
{
    Q_OBJECT
public:
    static CodeGeeXManager *instance();

    void checkCondaInstalled();
    bool condaHasInstalled();

    AbstractLLM *getCurrentLLM() const;

    void setLocale(CodeGeeX::Locale locale);

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

Q_SIGNALS:
    void sendSyncRequest(const QJsonObject &obj);
    void requestMessageUpdate(const MessageData &msg);
    void chatStarted();
    void crawledWebsite(const QString &msgID, const QList<websiteReference> &websites);
    void terminated();
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

public Q_SLOTS:
    void slotSendSyncRequest(const QJsonObject &obj);
    void onResponse(const QString &msgID, const QString &data, AbstractLLM::ResponseState state);
    void recevieDeleteResult(const QStringList &talkIds, bool success);
    void onLLMChanged(const LLMInfo &llmInfo);

private:
    explicit CodeGeeXManager(QObject *parent = nullptr);

    void initConnections();
    void initLLM(AbstractLLM *llm);
    QString modifiedData(const QString &data);

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
    AbstractLLM *liteLLM { nullptr }; //codegeex Lite.
    CodeGeeX::Locale locale { CodeGeeX::Zh };
};

#endif   // CODEGEEXMANAGER_H
