#ifndef SENDEVENTS_H
#define SENDEVENTS_H

#include <QString>

class SendEvents final
{
    SendEvents() = delete;
    SendEvents(const SendEvents &) = delete;
public:
    static void navRecentShow();
    static void navRecentHide();
    static void navEditHide();
    static void navEditShow();
    static void navDebugShow();
    static void NavDebugHide();
    static void navRuntimeShow();
    static void navRuntimeHide();
    static void menuOpenFile(const QString &path);
    static void menuOpenDirectory(const QString &path);
};

#endif // SENDEVENTS_H
