// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messagedata.h"

#include <QDebug>

MessageData::MessageData()
{
}

MessageData::MessageData(const QString &id, Type type)
    : msgId(id),
      msgType(type)
{
}

void MessageData::updateData(const QString &data)
{
    auto modifiedData = data;
    modifiedData.replace("\\n", "\n");
    modifiedData.replace("\\\"", "\"");

    QStringList lines = modifiedData.split("\n", QString::SkipEmptyParts);
    if (lines.length() < msgDataLines.length())
        return;

    msgData = modifiedData;
    msgDataLines = lines;
    //    qInfo() << "update msg line" << msgDataLines;
}

void MessageData::appendData(const QStringList &data)
{
    msgDataLines.append(data);
    msgData.append(data.join("\n"));
}

QString MessageData::messageID() const
{
    return msgId;
}

MessageData::Type MessageData::messageType() const
{
    return msgType;
}

QString MessageData::messageData() const
{
    return msgData;
}

QStringList MessageData::messageLines() const
{
    return msgDataLines;
}
