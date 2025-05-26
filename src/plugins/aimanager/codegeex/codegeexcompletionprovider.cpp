// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeexcompletionprovider.h"
#include "codegeexllm.h"

bool responseValid(const QString &response)
{
    bool valid = !(response.isEmpty()
                   || response.startsWith("\n\n\n")
                   || response.startsWith("\n    \n    "));
    if (!valid) {
        qWarning() << "Reponse not valid: " << response;
    }
    return valid;
}

CodeGeeXCompletionProvider::CodeGeeXCompletionProvider(QObject *parent)
    : AbstractInlineCompletionProvider(parent)
{
    completeLLM = new CodeGeeXLLM(this);
    completeLLM->setStream(false);
    connect(completeLLM, &AbstractLLM::dataReceived, this, &CodeGeeXCompletionProvider::handleDataReceived);

    timer.setSingleShot(true);
    timer.setInterval(500);
}

QString CodeGeeXCompletionProvider::providerName() const
{
    return "CodeGeeX";
}

void CodeGeeXCompletionProvider::provideInlineCompletionItems(const Position &pos, const InlineCompletionContext &c)
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
    connect(&timer, &QTimer::timeout, this, &CodeGeeXCompletionProvider::postGenerate, Qt::UniqueConnection);
    timer.start();
}

QList<AbstractInlineCompletionProvider::InlineCompletionItem> CodeGeeXCompletionProvider::inlineCompletionItems() const
{
    return completionItems;
}

void CodeGeeXCompletionProvider::setInlineCompletionEnabled(bool enabled)
{
    if (!enabled && timer.isActive())
        timer.stop();

    completionEnabled = enabled;
}

void CodeGeeXCompletionProvider::setInlineCompletions(const QStringList &completions)
{
    completionItems.clear();
    for (const auto &completion : completions) {
        InlineCompletionItem item { completion, positon };
        completionItems << item;
    }
}

bool CodeGeeXCompletionProvider::inlineCompletionEnabled() const
{
    return completionEnabled;
}

void CodeGeeXCompletionProvider::accepted()
{
    completionItems.clear();
}

void CodeGeeXCompletionProvider::rejected()
{
    completionItems.clear();
}

AbstractInlineCompletionProvider::InlineCompletionContext CodeGeeXCompletionProvider::inlineCompletionContext() const
{
    return context;
}

void CodeGeeXCompletionProvider::postGenerate()
{
    if (!inlineCompletionEnabled())
        return;

    const auto &context = inlineCompletionContext();
    if (!context.prefix.endsWith(generatedCode) || generateCache.isEmpty()) {
        generateType = checkPrefixType(context.prefix);
        completeLLM->generate(context.prefix, context.suffix);
    } else {
        generatedCode = extractSingleLine();
        setInlineCompletions({ generatedCode });
        emit finished();
    }
}

QString CodeGeeXCompletionProvider::extractSingleLine()
{
    if (generateCache.isEmpty())
        return "";

    bool extractedCode = false;
    QString completion = "";
    for (auto line : generateCache) {
        if (extractedCode)
            break;
        if (line != "")
            extractedCode = true;

        completion += line == "" ? "\n" : line;
        generateCache.removeFirst();
    }
    completion += "\n";

    //check if left cache all '\n'
    bool leftAllEmpty = true;
    for (auto line : generateCache) {
        if (line == "")
            continue;
        leftAllEmpty = false;
        break;
    }
    if (leftAllEmpty) {
        generateCache.clear();
        completion += "\n";
    }

    if (!extractedCode)
        completion = "";
    return completion;
}

CodeGeeXCompletionProvider::GenerateType CodeGeeXCompletionProvider::checkPrefixType(const QString &prefixCode)
{
    //todo
    Q_UNUSED(prefixCode)
    if (0)
        return CodeGeeXCompletionProvider::Line;
    else
        return CodeGeeXCompletionProvider::Block;
}

void CodeGeeXCompletionProvider::handleDataReceived(const QString &data, AbstractLLM::ResponseState state)
{
    if (state == AbstractLLM::Failed || state == AbstractLLM::Canceled)
        return;
    QString completion = "";
    if (generateType == Line) {
        generateCache = data.split('\n');
        completion = extractSingleLine();
    } else if (generateType == Block) {
        generateCache.clear();
        completion = data;
    }
    if (completion.endsWith('\n'))
        completion.chop(1);
    generatedCode = completion;
    setInlineCompletions({ completion });
    emit finished();
}
