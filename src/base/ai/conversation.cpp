// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "conversation.h"

#include <QJsonDocument>
#include <QVariant>

Conversation::Conversation()
{
}

Conversation::~Conversation()
{
}

QString Conversation::conversationLastUserData(const QString &conversation)
{
    const QJsonArray &array = QJsonDocument::fromJson(conversation.toUtf8()).array();
    if (!array.isEmpty() && array.last()["role"] == "user") {
        return array.last()["content"].toString();
    }

    return conversation;
}

bool Conversation::setSystemData(const QString &data)
{
    if (!data.isEmpty()) {
        for (auto iter = conversation.begin(); iter != conversation.end(); iter++) {
            if (iter->toObject().value("role").toString() == "system")
                return false;
        }
        conversation.insert(0, QJsonObject({ { "role", "system" }, {"content", data} }));
        return true;
    }
    return false;
}

bool Conversation::popSystemData()
{
    if (!conversation.isEmpty() && conversation.at(0)["role"].toString() == "system") {
        conversation.removeFirst();
        return true;
    }
    return false;
}

bool Conversation::addUserData(const QString &data)
{
    if (!data.isEmpty()) {
        const QJsonDocument &document = QJsonDocument::fromJson(data.toUtf8());
        if (document.isArray()) {
            conversation = document.array();
        } else {
            conversation.push_back(QJsonObject({ { "role", "user" }, {"content", data} }));
        }
        return true;
    }
    return false;
}

bool Conversation::popUserData()
{
    if (!conversation.isEmpty() && conversation.last()["role"].toString() == "user") {
        conversation.removeLast();
        return true;
    }
    return false;
}

QString Conversation::getLastResponse() const
{
    if (!conversation.isEmpty() && conversation.last()["role"].toString() == "assistant") {
        return conversation.last()["content"].toString();
    }
    return QString();
}

QByteArray Conversation::getLastByteResponse() const
{
    if (!conversation.isEmpty() && conversation.last()["role"].toString() == "assistant") {
        return conversation.last()["content"].toVariant().toByteArray();
    }
    return QByteArray();
}

bool Conversation::popLastResponse()
{
    if (!conversation.isEmpty() && conversation.last()["role"].toString() == "assistant") {
        conversation.removeLast();
        return true;
    }
    return false;
}

QJsonObject Conversation::getLastTools() const
{
    if (!conversation.isEmpty() && conversation.last()["role"].toString() == "tools") {
        return conversation.last()["content"].toObject();
    }

    return QJsonObject();
}

bool Conversation::popLastTools()
{
    if (!conversation.isEmpty() && conversation.last()["role"].toString() == "tools") {
        conversation.removeLast();
        return true;
    }
    return false;
}

bool Conversation::setFunctions(const QJsonArray &functions)
{
    this->functions = functions;
    return true;
}

QJsonArray Conversation::getConversions() const
{
    return conversation;
}

QJsonArray Conversation::getFunctions() const
{
    return functions;
}

QJsonArray Conversation::getFunctionTools() const
{
    QJsonArray tools;
    for (const QJsonValue &fun : functions) {
        QJsonObject tool;
        tool["type"] = "function";
        tool["function"] = fun;
        tools << tool;
    }

    return tools;
}

void Conversation::clear()
{
    conversation = QJsonArray();
    functions = QJsonArray();
}
