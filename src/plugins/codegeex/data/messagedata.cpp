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
    if (lines.last().isEmpty())
        lines.removeLast();

    if (lines.length() < msgDataLines.length())
        return;

//    QStringList newLines = lines.mid(msgDataLines.length());
//    for (auto line : newLines) {
//        if (line.startsWith("```")) {
//            CodeData data;
//            data.langueage = line.mid(3);
//            codeDataList.append(data);
//        }
//    }

    msgData = data;
    msgDataLines = lines;
//    qInfo() << "update msg line" << msgDataLines;
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
