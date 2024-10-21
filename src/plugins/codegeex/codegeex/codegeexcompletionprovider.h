// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXCOMPLETIONPROVIDER_H
#define CODEGEEXCOMPLETIONPROVIDER_H

#include "base/abstractinlinecompletionprovider.h"

#include <QTimer>

namespace CodeGeeX {
class CodeGeeXCompletionProvider : public AbstractInlineCompletionProvider
{
    Q_OBJECT
public:
    explicit CodeGeeXCompletionProvider(QObject *parent = nullptr);

    QString providerName() const override;
    void provideInlineCompletionItems(const Position &pos, const InlineCompletionContext &c) override;
    QList<InlineCompletionItem> inlineCompletionItems() const override;
    bool inlineCompletionEnabled() const override;

    InlineCompletionContext inlineCompletionContext() const;
    void setInlineCompletionEnabled(bool enabled);
    void setInlineCompletions(const QStringList &completions);

private:
    Position positon;
    InlineCompletionContext context;
    QList<InlineCompletionItem> completionItems;
    QAtomicInteger<bool> completionEnabled { false };
    QTimer timer;
};
}   // namespace CodeGeeX

#endif   // CODEGEEXCOMPLETIONPROVIDER_H
