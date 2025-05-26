// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openaicompletionprovider.h"
#include "openaicompatiblellm.h"

OpenAiCompletionProvider::OpenAiCompletionProvider(QObject *parent)
    : AbstractInlineCompletionProvider(parent)
{
    timer.setSingleShot(true);
    timer.setInterval(500);
}

QString OpenAiCompletionProvider::providerName() const
{
    return "OpenAiCompatible";
}

void OpenAiCompletionProvider::provideInlineCompletionItems(const Position &pos, const InlineCompletionContext &c)
{
    for (const auto &item : qAsConst(completionItems)) {
        if (c.prefix.endsWith(item.completion) && !item.completion.isEmpty()) {
            return;
        }
    }

    if (completeLLM->modelState() == AbstractLLM::Busy)
        completeLLM->cancel();
    positon = pos;
    context = c;
    connect(&timer, &QTimer::timeout, this, &OpenAiCompletionProvider::postGenerate, Qt::UniqueConnection);
    timer.start();
}

QList<AbstractInlineCompletionProvider::InlineCompletionItem> OpenAiCompletionProvider::inlineCompletionItems() const
{
    return completionItems;
}

void OpenAiCompletionProvider::setInlineCompletionEnabled(bool enabled)
{
    if (!enabled && timer.isActive())
        timer.stop();

    completionEnabled = enabled;
}

void OpenAiCompletionProvider::setInlineCompletions(const QStringList &completions)
{
    completionItems.clear();
    for (const auto &completion : completions) {
        InlineCompletionItem item { completion, positon };
        completionItems << item;
    }
}

void OpenAiCompletionProvider::setLLM(AbstractLLM *llm)
{
    if (!llm)
        return;
    if (completeLLM)
        disconnect(completeLLM, &AbstractLLM::dataReceived, nullptr, nullptr);
    completeLLM = llm;
    completeLLM->setStream(false);
    connect(completeLLM, &AbstractLLM::dataReceived, this, &OpenAiCompletionProvider::onDataReceived);
}

bool OpenAiCompletionProvider::inlineCompletionEnabled() const
{
    return completionEnabled;
}

void OpenAiCompletionProvider::accepted()
{
    completionItems.clear();
}

void OpenAiCompletionProvider::rejected()
{
    completionItems.clear();
}

AbstractInlineCompletionProvider::InlineCompletionContext OpenAiCompletionProvider::inlineCompletionContext() const
{
    return context;
}

void OpenAiCompletionProvider::onDataReceived(const QString &data, AbstractLLM::ResponseState state)
{
    if (state == AbstractLLM::Success || state == AbstractLLM::CutByLength) {
        QString completion = "";
        completion = data;
        if (completion.endsWith('\n'))
            completion.chop(1);
        setInlineCompletions({ completion });
        emit finished();
    }
}

void OpenAiCompletionProvider::postGenerate()
{
    if (!inlineCompletionEnabled())
        return;

    const auto &context = inlineCompletionContext();
    completeLLM->generate(context.prefix, context.suffix);
}
