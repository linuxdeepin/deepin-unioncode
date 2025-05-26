// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLECONVERSATION_H
#define COMPATIBLECONVERSATION_H

#include <base/ai/conversation.h>

class OpenAiCompatibleConversation : public Conversation
{
public:
    explicit OpenAiCompatibleConversation();

public:
    void update(const QByteArray &response);

public:
    static QJsonObject parseContentString(const QString &content);
};

#endif // COMPATIBLECONVERSATION_H