// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MESSAGEDATA_H
#define MESSAGEDATA_H

#include <QString>
#include <QList>

struct CodeData {
    QString langueage {};
    QString code {};
    QStringList codeLines {};
};

class MessageData
{
public:
    enum Type{
        Ask,
        Anwser
    };
    explicit MessageData();
    explicit MessageData(const QString &id, Type type);

    void updateData(const QString &data);
    void appendData(const QStringList &data);

    QString messageID() const;
    Type messageType() const;
    QString messageData() const;
    QStringList messageLines() const;

private:
    QString msgId;
    Type msgType;
    QString msgData;
    QList<CodeData> codeDataList {};
    QStringList msgDataLines {};
};

#endif // MESSAGEDATA_H
