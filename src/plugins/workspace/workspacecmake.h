#ifndef WORKSPACECMAKE_H
#define WORKSPACECMAKE_H

#include "workspaceobject.h"

class WorkspaceCMake : public WorkspaceObject
{
public:
    WorkspaceCMake();
    static QString buildSystemName(){return "cmake";}
    virtual void generate(const QString &rootPath, const QString &targetPath);
};

#endif // WORKSPACECMAKE_H
