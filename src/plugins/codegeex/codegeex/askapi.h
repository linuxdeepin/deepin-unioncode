// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef ASKAPI_H
#define ASKAPI_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace CodeGeeX {
class AskApi : public QObject
{
    Q_OBJECT
public:
    explicit AskApi(QObject *parent = nullptr);

    void sendLoginRequest(const QString &sessionId,
                          const QString &machineId,
                          const QString &userId,
                          const QString &env = "deepin-unioncode");

    void sendQueryRequest(const QString &codeToken);

    void postSSEChat(const QString &url,
                     const QString &token,
                     const QString &prompt,
                     const QString &machineId,
                     const QMultiMap<QString, QString> &history);

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

    struct ChatRecord
    {
        QString prompt;
        QString outputText;
        QString talkId;
    };
    void getChatRecordByTalkId(const QString &url,
                               const QString &token,
                               const QString &talkId,
                               int pageNumber = 1,
                               int pageSize = 10);

    void deleteSessions(const QString &url,
                        const QString &token,
                        const QStringList &talkIds);

    enum LoginState {
        kLoginFailed,
        kLoginSuccess
    };

signals:
    void loginState(LoginState loginState);
    void response(const QString &msgID, const QString &response, const QString &event);
    void getSessionListResult(const QVector<SessionRecord> &records);
    void getChatRecordResult(const QVector<ChatRecord> &record);
    void sessionDeleted(const QStringList &talkId, bool isSuccessful);
    void sessionCreated(const QString &talkId, bool isSuccessful);
    void stopReceive();

public slots:

private:
    QNetworkReply *postMessage(const QString &url, const QString &token, const QByteArray &body);
    QNetworkReply *getMessage(const QString &url, const QString &token);
    void processResponse(QNetworkReply *reply);

    QByteArray assembleSSEChatBody(const QString &prompt,
                                   const QString &machineId,
                                   const QJsonArray &history);

    QByteArray assembleNewSessionBody(const QString &prompt,
                                      const QString &talkId);

    QByteArray assembleDelSessionBody(const QStringList &talkIds);

    QByteArray jsonToByteArray(const QJsonObject &jsonObject);
    QJsonObject toJsonOBject(QNetworkReply *reply);

    QNetworkAccessManager *manager = nullptr;
};
}   // end namespace

#endif   // ASKAPI_H
