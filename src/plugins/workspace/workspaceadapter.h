#ifndef WORKSPACEADAPTER_H
#define WORKSPACEADAPTER_H

#include <framework/framework.h>

#include <QString>

class WorkspaceObject
{
public:
    WorkspaceObject(){}
    virtual ~WorkspaceObject(){}
    static QString buildSystemName(){return "";}
    virtual void generate(const QString &rootPath, const QString &targetPath){
        Q_UNUSED(rootPath)
        Q_UNUSED(targetPath)
    }
};

class WorkspaceObjectFactory : public dpf::QtClassFactory<WorkspaceObject>
{
public:
    static WorkspaceObjectFactory *globalInstance(){
        static WorkspaceObjectFactory ins;
        return &ins;
    }
};

#endif // WORKSPACEADAPTER_H
