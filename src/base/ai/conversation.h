// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <QJsonObject>
#include <QJsonArray>

class Conversation
{
public:
    Conversation();
    virtual ~Conversation();

    /**
     * @brief conversationLastUserData
     * @param conversation
     * @return
     */
    static QString conversationLastUserData(const QString &conversation);

public:
    bool setSystemData(const QString &data);
    bool popSystemData();

    bool addUserData(const QString &data);
    bool popUserData();

    bool addResponse(const QString &data);
    QString getLastResponse() const;
    QByteArray getLastByteResponse() const;
    bool popLastResponse();

    QJsonObject getLastTools() const;
    bool popLastTools();

    bool setFunctions(const QJsonArray &functions);
    QJsonArray getFunctions() const;
    QJsonArray getFunctionTools() const;

    QJsonArray getConversions() const;

    void clear();
protected:
    QJsonArray conversation;
    QJsonArray functions;
};

#endif // CONVERSATION_H
