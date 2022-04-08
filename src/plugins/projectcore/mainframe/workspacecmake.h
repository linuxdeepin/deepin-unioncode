#ifndef WORKSPACECMAKE_H
#define WORKSPACECMAKE_H

#include "workspaceobject.h"

class WorkspaceCMakePrivate;
class WorkspaceCMake : public WorkspaceObject
{
    Q_OBJECT
    WorkspaceCMakePrivate *const d;
public:
    WorkspaceCMake();
    virtual ~WorkspaceCMake();
    static QString buildSystemName(){return "cmake";}
    virtual bool generate(const QString &rootPath, const QString &targetPath);
};

#endif // WORKSPACECMAKE_H
