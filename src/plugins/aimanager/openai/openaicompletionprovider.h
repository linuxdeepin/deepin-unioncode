// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENAICOMPLETIONPROVIDER_H
#define OPENAICOMPLETIONPROVIDER_H

#include "base/abstractinlinecompletionprovider.h"
#include "base/ai/abstractllm.h"

#include <QTimer>

class LLMInfo;
class OpenAiCompletionProvider : public AbstractInlineCompletionProvider
{
    Q_OBJECT
public:
    explicit OpenAiCompletionProvider(QObject *parent = nullptr);

    QString providerName() const override;
    void provideInlineCompletionItems(const Position &pos, const InlineCompletionContext &c) override;
    QList<InlineCompletionItem> inlineCompletionItems() const override;
    bool inlineCompletionEnabled() const override;
    void accepted() override;
    void rejected() override;

    InlineCompletionContext inlineCompletionContext() const;
    void setInlineCompletionEnabled(bool enabled);
    void setInlineCompletions(const QStringList &completions);
    void setLLM(AbstractLLM *llm);

private slots:
    void onDataReceived(const QString &data, AbstractLLM::ResponseState state);

private:
    void postGenerate();
    Position positon;
    InlineCompletionContext context;
    QList<InlineCompletionItem> completionItems;
    QAtomicInteger<bool> completionEnabled { false };
    QTimer timer;

    AbstractLLM *completeLLM { nullptr };
};

#endif   // OPENAICOMPLETIONPROVIDER_H
