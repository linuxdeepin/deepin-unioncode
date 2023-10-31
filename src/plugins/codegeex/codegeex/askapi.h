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
            const QString &taskId
            );

    enum LoginState
    {
        kLoginFailed,
        kLoginSuccess
    };

signals:
    void loginState(LoginState loginState);
    void response(const QString &response, const QString &event);

public slots:

private:
    QNetworkReply *postMessage(const QString &url, const QString &token, const QByteArray &body);
    void processResponse(QNetworkReply *reply);

    QByteArray assembleSSEChatBody(const QString &prompt,
                                   const QString &machineId,
                                   const QJsonArray &history);

    QByteArray assembleNewSessionBody(const QString &prompt,
                                      const QString &taskId);


    QByteArray jsonToByteArray(const QJsonObject &jsonObject);

    QNetworkAccessManager *manager = nullptr;
};
} // end namespace

#endif // ASKAPI_H
