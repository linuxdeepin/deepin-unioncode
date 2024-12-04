// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTLLM_H
#define ABSTRACTLLM_H

#include "conversation.h"

#include <QObject>
#include <QNetworkReply>

class AbstractLLM : public QObject
{
    Q_OBJECT
public:
    enum ResponseState {
        Receiving,
        Success,
        CutByLength,
        Failed,
        Canceled
    };

    explicit AbstractLLM(QObject *parent = nullptr);
    virtual ~AbstractLLM() {}

    virtual QString modelPath() const = 0;
    virtual bool checkValid(QString *errStr) = 0;
    virtual QJsonObject create(const Conversation &conversation) = 0;
    virtual void request(const QJsonObject &data) = 0;
    virtual void request(const QString &prompt) = 0;
    virtual void generate(const QString &prompt, const QString &suffix) = 0;
    virtual void setTemperature(double temperature) = 0;
    virtual void setStream(bool isStream) = 0;
    virtual void processResponse(QNetworkReply *reply) = 0;
    virtual void cancel() = 0;
    virtual void setMaxTokens(int maxToken) = 0;
    virtual Conversation *getCurrentConversation() = 0;

signals:
    void dataReceived(const QString &data, ResponseState statu);
};

#endif
