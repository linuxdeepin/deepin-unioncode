// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef ASKAPI_H
#define ASKAPI_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace CodeGeeX {
struct websiteReference
{
    QString citation;
    QString url;
    QString title;
    QString status;
};

struct Entry
{
    QString type;
    QString text;
    QList<websiteReference> websites;
};

class AskApiPrivate;
class AskApi : public QObject
{
    Q_OBJECT
public:
    struct SessionRecord
    {
        QString prompt;
        QString talkId;
        QString createdTime;
    };

    struct MessageRecord
    {
        QString input;
        QString output;
    };

    enum LoginState {
        kLoginFailed,
        kLoginSuccess,
        kLoginOut
    };

    explicit AskApi(QObject *parent = nullptr);
    ~AskApi();

    void sendLoginRequest(const QString &sessionId,
                          const QString &machineId,
                          const QString &userId,
                          const QString &env = "deepin-unioncode");

    void logout(const QString &codeToken);

    void sendQueryRequest(const QString &codeToken);

    void postSSEChat(const QString &url,
                     const QString &token,
                     const QString &prompt,
                     const QString &machineId,
                     const QMultiMap<QString, QString> &history,
                     const QString &talkId);

    void postNewSession(const QString &url,
                        const QString &token,
                        const QString &prompt,
                        const QString &talkId);

    void getSessionList(const QString &url,
                        const QString &token,
                        int pageNumber = 1,
                        int pageSize = 10);

    void getMessageList(const QString &url,
                        const QString &token,
                        int pageNumber = 1,
                        int pageSize = 10,
                        const QString &talkId = "");

    void deleteSessions(const QString &url,
                        const QString &token,
                        const QStringList &talkIds);

    void setModel(const QString &model);
    void setLocale(const QString &locale);
    void setNetworkEnabled(bool enabled);
    bool networkEnabled() const;
    void setReferenceFiles(const QStringList &fileList);
    QStringList referenceFiles() const;
    void setCodebaseEnabled(bool enabled);
    bool codebaseEnabled() const;

signals:
    void loginState(LoginState loginState);
    void response(const QString &msgID, const QString &response, const QString &event);
    void crawledWebsite(const QString &msgID, const QList<websiteReference> &websites);
    void getSessionListResult(const QVector<SessionRecord> &records);
    void getMessageListResult(const QVector<MessageRecord> &records);
    void sessionDeleted(const QStringList &talkId, bool isSuccessful);
    void sessionCreated(const QString &talkId, bool isSuccessful);
    void stopReceive();
    void syncSendMessage(const QString url, const QString &token, const QByteArray &body);
    void noChunksFounded();
    void notify(int type, const QString &message);

public slots:
    void slotSendMessage(const QString url, const QString &token, const QByteArray &body);

private:
    AskApiPrivate *const d;
};
}   // end namespace

#endif   // ASKAPI_H
