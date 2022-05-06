#include "sendevents.h"
#include "framework/framework.h"
#include "common/common.h"

void SendEvents::generateStart(const QString &buildSystem, const QString &projectPath, const QString &targetPath)
{
    dpf::Event event;
    event.setTopic(T_WORKSPACE);
    event.setData(D_WORKSPACE_GENERATE_BEGIN);
    event.setProperty(P_BUILDSYSTEM, buildSystem);
    event.setProperty(P_PROJECTPATH, projectPath);
    event.setProperty(P_TARGETPATH, targetPath);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void SendEvents::gengrateEnd(const QString &buildSystem, const QString &projectPath, const QString &targetPath)
{
    dpf::Event event;
    event.setTopic(T_WORKSPACE);
    event.setData(D_WORKSPACE_GENERATE_END);
    event.setProperty(P_BUILDSYSTEM, buildSystem);
    event.setProperty(P_PROJECTPATH, projectPath);
    event.setProperty(P_TARGETPATH, targetPath);
    dpf::EventCallProxy::instance().pubEvent(event);
}

void SendEvents::doubleCliekedOpenFile(const QString &filePath, const QString &workspaceFolder)
{
    dpf::Event event;
    event.setTopic(T_PROJECT);
    event.setData(D_ITEM_DOUBLECLICKED);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_WORKSPACEFOLDER, workspaceFolder);
    dpf::EventCallProxy::instance().pubEvent(event);
}
