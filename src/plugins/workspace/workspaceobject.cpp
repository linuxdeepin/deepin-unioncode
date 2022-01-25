#include "workspaceobject.h"

WorkspaceObjectFactory *WorkspaceObjectFactory::globalInstance()
{
    static WorkspaceObjectFactory ins;
    return &ins;
}

QString WorkspaceObjectFactory::findkey(WorkspaceObject *ins)
{
    return classList.key(ins);
}

WorkspaceObject *WorkspaceObjectFactory::create(const QString &name, QString *errorString)
{
    WorkspaceObject* cache = globalInstance()->value(name);
    if (!cache){
        cache = globalInstance()->dpf::QtClassFactory<WorkspaceObject>
                ::create(name, errorString);
        globalInstance()->append(name, cache, errorString);
    }
    return cache;
}

QString WorkspaceObjectFactory::key(WorkspaceObject *ins)
{
    return globalInstance()->findkey(ins);
}

WorkspaceObject::WorkspaceObject()
{

}

WorkspaceObject::~WorkspaceObject()
{

}

bool WorkspaceObject::generate(const QString &rootPath, const QString &targetPath)
{
    Q_UNUSED(rootPath)
    Q_UNUSED(targetPath)
    // emit generateStart(rootPath, targetPath);
    // do something
    // emit generateEnd(rootPath, targetPath);
    return false;
}
