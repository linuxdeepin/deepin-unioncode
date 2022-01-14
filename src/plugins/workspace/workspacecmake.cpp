#include "workspacecmake.h"

WorkspaceCMake::WorkspaceCMake()
{

}

void WorkspaceCMake::generate(const QString &rootPath, const QString &targetPath)
{
    qInfo() << rootPath << targetPath;
}
