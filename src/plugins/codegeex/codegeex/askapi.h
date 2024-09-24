// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef ASKAPI_H
#define ASKAPI_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace CodeGeeX {
struct websiteReference {
    QString citation;
    QString url;
    QString title;
    QString status;
};

struct Entry {
    QString type;
    QString text;
    QList<websiteReference> websites;
};

class AskApi : public QObject
{
    Q_OBJECT
public:
    explicit AskApi(QObject *parent = nullptr);

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

    void postNewSession(
            const QString &url,
            const QString &token,
            const QString &prompt,
            const QString &talkId);

    struct SessionRecord
    {
        QString prompt;
        QString talkId;
        QString createdTime;
    };
    void getSessionList(const QString &url,
                        const QString &token,
                        int pageNumber = 1,
                        int pageSize = 10);

    struct MessageRecord
    {
        QString input;
        QString output;
    };
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
    enum LoginState {
        kLoginFailed,
        kLoginSuccess,
        kLoginOut
    };

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

public slots:
    void slotSendMessage(const QString url, const QString &token, const QByteArray &body);

private:
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

    QNetworkAccessManager *manager = nullptr;
    QString model = "codegeex-chat-lite";
    QString locale = "zh";
};
}   // end namespace

#endif   // ASKAPI_H
