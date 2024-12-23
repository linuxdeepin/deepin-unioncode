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
    enum LLMState {
        Idle = 0,
        Busy
    };

    enum ResponseState {
        Receiving,
        Success,
        CutByLength,
        Failed,
        Canceled
    };

    enum Locale {
        Zh,
        En
    };

    using ResponseHandler = std::function<void(const QString &data, ResponseState state)>;

    explicit AbstractLLM(QObject *parent = nullptr);
    virtual ~AbstractLLM() {}

    virtual QString modelName() const = 0;
    virtual QString modelPath() const = 0;
    virtual bool checkValid(QString *errStr) = 0;
    virtual QJsonObject create(const Conversation &conversation) = 0;
    virtual void request(const QJsonObject &data) = 0;
    virtual void request(const QString &prompt, ResponseHandler handler = nullptr) = 0;
    virtual void generate(const QString &prefix, const QString &suffix) = 0; // use to auto compelte
    virtual void setTemperature(double temperature) = 0;
    virtual void setStream(bool isStream) = 0;
    virtual void setLocale(Locale lc) = 0;
    virtual void cancel() = 0;
    virtual void setMaxTokens(int maxToken) = 0;
    virtual Conversation *getCurrentConversation() = 0;

    void setModelState(LLMState st);
    LLMState modelState() const;

signals:
    void customDataReceived(const QString &key, const QJsonObject &customData);
    void dataReceived(const QString &data, ResponseState state);
    void modelStateChanged();

private:
    QAtomicInt state { Idle };
};

#endif
