// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXCOMPLETIONPROVIDER_H
#define CODEGEEXCOMPLETIONPROVIDER_H

#include "base/abstractinlinecompletionprovider.h"
#include "base/ai/abstractllm.h"

#include <QTimer>

class CodeGeeXCompletionProvider : public AbstractInlineCompletionProvider
{
    Q_OBJECT
public:
    enum GenerateType {
        Line,
        Block
    };

    explicit CodeGeeXCompletionProvider(QObject *parent = nullptr);

    QString providerName() const override;
    void provideInlineCompletionItems(const Position &pos, const InlineCompletionContext &c) override;
    QList<InlineCompletionItem> inlineCompletionItems() const override;
    bool inlineCompletionEnabled() const override;
    void accepted() override;
    void rejected() override;

    InlineCompletionContext inlineCompletionContext() const;
    void setInlineCompletionEnabled(bool enabled);
    void setInlineCompletions(const QStringList &completions);

private:
    void postGenerate();
    Position positon;
    InlineCompletionContext context;
    QList<InlineCompletionItem> completionItems;
    QAtomicInteger<bool> completionEnabled { false };
    void handleDataReceived(const QString &data, AbstractLLM::ResponseState state);
    QTimer timer;

    QStringList generateCache {};
    QString generatedCode {};
    QString extractSingleLine();

    AbstractLLM *completeLLM { nullptr };
    GenerateType generateType;
    GenerateType checkPrefixType(const QString &prefixCode);
};

#endif   // CODEGEEXCOMPLETIONPROVIDER_H
