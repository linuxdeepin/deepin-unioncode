// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openaicompatibleconversation.h"

#include <QJsonDocument>
#include <QRegularExpression>

void mergeFunctionCall(QJsonObject &functionCall, const QJsonObject &delta)
{
    if (delta.contains("name")) {
        functionCall["name"] = functionCall["name"].toString() + delta.value("name").toString();
    }
    if (delta.contains("arguments")) {
        functionCall["arguments"] = functionCall["arguments"].toString() + delta.value("arguments").toString();
    }
}

void mergeToolCallMap(QMap<int, QJsonObject> &toolCallMaps, const QJsonArray &tool_calls)
{
    for (const QJsonValue &tool_call : tool_calls) {
        const QJsonObject &toolCallObj = tool_call.toObject();
        int index = toolCallObj["index"].toInt();

        if (!toolCallMaps[index].contains("function")) {
            toolCallMaps[index]["function"] = QJsonObject();
        }

        toolCallMaps[index]["index"] = index;

        if (toolCallObj.contains("id")) {
            toolCallMaps[index]["id"] = toolCallObj.value("id");
        }
        if (toolCallObj.contains("type")) {
            toolCallMaps[index]["type"] = toolCallObj.value("type");
        }

        QJsonObject toolFun = toolCallMaps[index]["function"].toObject();

        if (const QJsonValue &tmpToolFunVal = toolCallObj.value("function"); !tmpToolFunVal.isUndefined()) {
            const QJsonObject &tmpToolFun = tmpToolFunVal.toObject();
            if (tmpToolFun.contains("name")) {
                toolFun["name"] = toolFun["name"].toString() + tmpToolFun.value("name").toString();
            }
            if (tmpToolFun.contains("arguments")) {
                toolFun["arguments"] = toolFun["arguments"].toString() + tmpToolFun.value("arguments").toString();
            }
        }

        toolCallMaps[index]["function"] = toolFun;
    }
}


OpenAiCompatibleConversation::OpenAiCompatibleConversation()
{
}

QJsonObject OpenAiCompatibleConversation::parseContentString(const QString &content)
{
    QString deltacontent;
    QRegularExpression regex(R"(data:\s*\{(.*)\})");
    QRegularExpressionMatchIterator iter = regex.globalMatch(content);
    QString finishReason;
    QJsonObject functionCall;
    QMap<int, QJsonObject> toolCallMaps;

    while (iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        QString matchString = match.captured(0);
        int startIndex = matchString.indexOf('{');
        int endIndex = matchString.lastIndexOf('}');
        if (startIndex < 0 || endIndex <= startIndex) {
            continue;
        }
        QString content = matchString.mid(startIndex, endIndex - startIndex + 1);
        QJsonObject j = QJsonDocument::fromJson(content.toUtf8()).object();
        
        if (!j.contains("choices")) {
            continue;
        }

        const QJsonArray &choices = j["choices"].toArray();
        for (const QJsonValue &choice : choices) {
            const QJsonObject &cj = choice.toObject();

            if (cj.contains("finish_reason")) {
                finishReason = cj["finish_reason"].toString();
            }

            if (cj.contains("delta")) {
                const QJsonObject &delta = cj["delta"].toObject();

                if (delta.contains("content")) {
                    deltacontent += delta["content"].toString();
                }

                if (delta.contains("function_call")) {
                    mergeFunctionCall(functionCall, delta.value("function_call").toObject());
                }

                if (delta.contains("tool_calls")) {
                    mergeToolCallMap(toolCallMaps, delta.value("tool_calls").toArray());
                }
            } else if (cj.contains("text")) {
                deltacontent += cj["text"].toString();
            }
        }
    }

    QJsonObject response;
    if (!deltacontent.isEmpty()) {
        response["content"] = deltacontent;
    }

    if (!functionCall.isEmpty() || !toolCallMaps.isEmpty()) {
        QJsonObject tools;

        if (!functionCall.isEmpty()) {
            tools["function_call"] = functionCall;
        }

        if (!toolCallMaps.isEmpty()) {
            QJsonArray toolCalls;
            for (const auto &toolCallObj : toolCallMaps) {
                toolCalls << toolCallObj;
            }
            tools["tool_calls"] = toolCalls;
        }

        response["tools"] = tools;
    }

    if (!finishReason.isEmpty()) {
        response["finish_reason"] = finishReason;
    }

    return response;
}

void OpenAiCompatibleConversation::update(const QByteArray &response)
{
    if (response.isEmpty())
        return;

    if (response.startsWith("data:")) {
        const QJsonObject &delateData = parseContentString(response);
        if (delateData.contains("content")) {
            conversation.push_back(QJsonObject({
                { "role",     "assistant"    },
                { "content",  delateData.value("content") }
            }));
        }
    } else {
        const QJsonObject &j = QJsonDocument::fromJson(response).object();
        if (j.contains("choices")) {
            const QJsonArray &choices = j["choices"].toArray();
            for (auto choice = choices.begin(); choice != choices.end(); choice++) {
                const QJsonObject &cj = choice->toObject();
                if (cj.contains("message")) {
                    if (!cj["message"]["role"].isNull() && !cj["message"]["content"].isNull()) {
                        conversation.push_back(QJsonObject({
                            { "role",    cj["message"]["role"]    },
                            { "content", cj["message"]["content"] }
                        }));
                    }
                }
            }
        } else if (j.contains("message")) {
            if (!j["message"]["role"].isNull() && !j["message"]["content"].isNull()) {
                conversation.push_back(QJsonObject({
                    { "role",    j["message"]["role"]    },
                    { "content", j["message"]["content"] }
                }));
            }
        } else if (j.contains("role") && j.contains("content")) {
            conversation.push_back(QJsonObject({
                { "role",    j["role"]    },
                { "content", j["content"] }
            }));
        }
    }
}
