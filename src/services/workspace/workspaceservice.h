#ifndef WORKSPACESERVICE_H
#define WORKSPACESERVICE_H

#include <framework/framework.h>

namespace dpfservice {
class WorkspaceService final : public dpf::PluginService, dpf::AutoServiceRegister<WorkspaceService>
{
    Q_OBJECT
    Q_DISABLE_COPY(WorkspaceService)
public:
    static QString name();
    explicit WorkspaceService(QObject *parent = nullptr);
    static QString workspaceName(const QString &dirPath);
    DPF_INTERFACE(QString, targetPath, const QString &);
    DPF_INTERFACE(QStringList, findWorkspace, const QString &);

signals:
    void addWorkspace(const QString &);
    void delWorkspace(const QString &);
};
}

#endif // WORKSPACESERVICE_H
