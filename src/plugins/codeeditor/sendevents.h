#ifndef SENDEVENTS_H
#define SENDEVENTS_H

#include <QString>

class SendEvents final
{
    SendEvents() = delete;
    SendEvents(const SendEvents &) = delete;
public:
    static void marginDebugPointAdd(const QString filePath, quint64 fileLine);
    static void marginDebugPointRemove(const QString filePath, quint64 fileLine);
};

#endif // SENDEVENTS_H
