// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openaicompatibleconversation.h"

#include <QJsonDocument>
#include <QRegularExpression>

OpenAiCompatibleConversation::OpenAiCompatibleConversation()
{
}

QJsonObject OpenAiCompatibleConversation::parseContentString(const QString &content)
{
    QString deltacontent;

    QRegularExpression regex(R"(data:\s*\{(.*)\})");
    QRegularExpressionMatchIterator iter = regex.globalMatch(content);

    QString finishReason = "";
    while (iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        QString matchString = match.captured(0);

        int startIndex = matchString.indexOf('{');
        int endIndex = matchString.lastIndexOf('}');

        if (startIndex >= 0 && endIndex > startIndex) {
            QString content = matchString.mid(startIndex, endIndex - startIndex + 1);

            QJsonObject j = QJsonDocument::fromJson(content.toUtf8()).object();
            if (j.contains("choices")) {
                const QJsonArray &choices = j["choices"].toArray();
                for (auto choice = choices.begin(); choice != choices.end(); choice++) {
                    const QJsonObject &cj = choice->toObject();
                    if (cj.contains("finish_reason"))
                        finishReason = cj["finish_reason"].toString();
                    if (cj.contains("delta")) {
                        const QJsonObject &delta = cj["delta"].toObject();
                        if (delta.contains("content")) {
                            const QString &deltaData = delta["content"].toString();
                            deltacontent += deltaData;
                        }
                    } else if (cj.contains("text")) {
                        deltacontent += cj["text"].toString();
                    }
                }
            }
        }
    }

    QJsonObject response;
    if (!deltacontent.isEmpty()) {
        response["content"] = deltacontent;
    }

    if (!finishReason.isEmpty())
        response["finish_reason"] = finishReason;

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
