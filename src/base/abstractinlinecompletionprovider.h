// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTINLINECOMPLETIONPROVIDER_H
#define ABSTRACTINLINECOMPLETIONPROVIDER_H

#include <QObject>

class AbstractInlineCompletionProvider : public QObject
{
    Q_OBJECT
public:
    struct Position
    {
        int line = -1;
        int column = -1;

        bool operator==(const Position &pos) const
        {
            return line == pos.line && column == pos.column;
        }

        bool operator!=(const Position &pos) const
        {
            return !(operator==(pos));
        }
    };

    struct InlineCompletionContext
    {
        QString prefix;
        QString suffix;
    };

    struct InlineCompletionItem
    {
        QString completion;
        Position pos;
    };

    explicit AbstractInlineCompletionProvider(QObject *parent = nullptr);

    virtual QString providerName() const = 0;
    virtual void provideInlineCompletionItems(const Position &pos, const InlineCompletionContext &contex) = 0;
    virtual QList<InlineCompletionItem> inlineCompletionItems() const = 0;
    virtual bool inlineCompletionEnabled() const { return false; }

Q_SIGNALS:
    void finished();
};

Q_DECLARE_METATYPE(AbstractInlineCompletionProvider::InlineCompletionItem)
#endif   // ABSTRACTINLINECOMPLETIONPROVIDER_H
