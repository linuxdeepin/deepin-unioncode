#include "workspaceservice.h"

namespace dpfservice {
QString dpfservice::WorkspaceService::name()
{
    return "org.deepin.service.WorkspaceService";;
}

WorkspaceService::WorkspaceService(QObject *parent)
    : dpf::PluginService (parent)
{

}

QString WorkspaceService::workspaceName(const QString &dirPath)
{
    return QFileInfo(dirPath).fileName();
}

} // namespace dpfservice
