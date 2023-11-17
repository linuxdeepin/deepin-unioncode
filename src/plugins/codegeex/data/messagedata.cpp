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
    QStringList lines = data.split("\n");
    lines.removeAll("");

    if (lines.length() < msgDataLines.length())
        return;

    msgData = data;
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
