// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <base/ai/abstractllm.h>

#ifndef CodeGeeXLLM_H
#define CodeGeeXLLM_H

struct Entry
{
    QString type;
    QString text;
    QJsonObject websites;
};

class CodeGeeXLLMPrivate;
class CodeGeeXLLM : public AbstractLLM
{
    Q_OBJECT
public:
    explicit CodeGeeXLLM(QObject *parent = nullptr);
    ~CodeGeeXLLM() override;

    Conversation* getCurrentConversation() override;
    void setModelName(const QString &modelName);
    void setModelPath(const QString &path);
    void setApiKey(const QString &apiKey);

    QString modelName() const override;
    QString modelPath() const override;
    bool checkValid(QString *errStr) override;
    QJsonObject create(const Conversation &conversation) override;
    void request(const QJsonObject &data) override;
    void request(const QString &prompt, ResponseHandler handler = nullptr) override;
    void generate(const QString &prefix, const QString &suffix) override;
    void setTemperature(double temperature) override;
    void setStream(bool isStream) override;
    void setLocale(Locale lc) override;
    void cancel() override;
    void setMaxTokens(int maxTokens) override;

signals:
    void requstCancel();

private:
    CodeGeeXLLMPrivate *d;
};

#endif // CodeGeeXLLM_H
