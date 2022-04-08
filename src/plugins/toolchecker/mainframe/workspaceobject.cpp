#include "workspaceobject.h"

WorkspaceObjectFactory *WorkspaceObjectFactory::globalInstance()
{
    static WorkspaceObjectFactory ins;
    return &ins;
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
