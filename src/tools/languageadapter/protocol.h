#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

namespace protocol{

inline QString setHeader(const QJsonObject &object)
{
    QJsonDocument jsonDoc(object);
    QString jsonStr = jsonDoc.toJson();
    return "Content-Length" + QString(": %0\r\n\r\n").arg(jsonStr.length()) + jsonStr;
}

} //namespace protocol

#endif // PROTOCOL_H
