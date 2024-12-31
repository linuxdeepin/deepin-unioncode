// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <base/ai/abstractllm.h>

#ifndef OPENAICOMPATIBLELLM_H
#define OPENAICOMPATIBLELLM_H

class OpenAiCompatibleLLMPrivate;
class OpenAiCompatibleLLM : public AbstractLLM
{
    Q_OBJECT
public:
    explicit OpenAiCompatibleLLM(QObject *parent = nullptr);
    ~OpenAiCompatibleLLM() override;

    Conversation* getCurrentConversation() override;
    void setModelName(const QString &modelName);
    void setModelPath(const QString &path);
    void setApiKey(const QString &apiKey);

    QString modelName() const override;
    QString modelPath() const override;
    bool checkValid(QString *errStr) override;
    QJsonObject create(const Conversation &conversation) override;
    void request(const QJsonObject &data) override; // v1/chat/compltions
    void request(const QString &prompt) override; // v1/completions
    void generate(const QString &prompt, const QString &suffix) override; // api/generate
    void setTemperature(double temperature) override;
    void setStream(bool isStream) override;
    void processResponse(QNetworkReply *reply) override;
    void cancel() override;
    void setMaxTokens(int maxTokens) override;
    bool isIdle() override;

signals:
    void requstCancel();

private:
    OpenAiCompatibleLLMPrivate *d;
};

#endif // OPENAICOMPATIBLELLM_H
