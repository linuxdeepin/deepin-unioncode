#include "workspacedata.h"
#include <QSet>
#include <QFileInfo>
#include <QFile>
#include <QDir>

class WorkspaceDataPrivate
{
    friend class WorkspaceData;
    QSet<QString> workspaceFolders;
};

WorkspaceData::WorkspaceData(QObject *parent)
    : QObject(parent)
    , d(new WorkspaceDataPrivate)
{

}

WorkspaceData::~WorkspaceData()
{
    if (d) {
        if (!d->workspaceFolders.isEmpty())
            d->workspaceFolders.clear();
        delete d;
    }
}

WorkspaceData *WorkspaceData::globalInstance() {
    static WorkspaceData ins;
    return &ins;
}

void WorkspaceData::addWorkspace(const QString &dirPath)
{
    d->workspaceFolders.insert(dirPath);
}

void WorkspaceData::delWorkspace(const QString &dirPath)
{
    d->workspaceFolders.remove(dirPath);
}

QStringList WorkspaceData::findWorkspace(const QString &filePath)
{
    QStringList result;
    QFileInfo info(filePath);
    if (info.isFile()) {
        info = QFileInfo(info.dir().absolutePath());
    }

    if (info.isDir()) {
        if (d->workspaceFolders.contains(info.filePath())) {
            result << info.filePath();
        }

        QDir dir(info.filePath());
        while(!dir.cdUp()) {
            if (d->workspaceFolders.contains(dir.absolutePath())) {
                result << dir.absolutePath();
            }
        }
    }
    return result;
}
