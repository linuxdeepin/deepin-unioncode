#ifndef SENDEVENTS_H
#define SENDEVENTS_H

#include <QString>
#include "services/project/projectservice.h"

class SendEvents final
{
    SendEvents() = delete;
    SendEvents(const SendEvents &) = delete;
public:
    static void generateStart(const QString &buildSystem, const QString &projectPath, const QString &targetPath);
    static void gengrateEnd(const QString &buildSystem,const QString &projectPath, const QString &targetPath);
    static void doubleCliekedOpenFile(const QString &workspace, const QString &language, const QString &filePath);
    static void projectActived(const dpfservice::ProjectInfo &info);
    static void projectCreated(const dpfservice::ProjectInfo &info);
    static void projectDeleted(const dpfservice::ProjectInfo &info);
};

#endif // SENDEVENTS_H
