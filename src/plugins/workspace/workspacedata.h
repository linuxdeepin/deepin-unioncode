#ifndef WORKSPACEDATA_H
#define WORKSPACEDATA_H

#include <QObject>
class WorkspaceDataPrivate;
class WorkspaceData : public QObject
{
    Q_OBJECT
    WorkspaceDataPrivate *const d;
public:
    explicit WorkspaceData(QObject *parent = nullptr);
    virtual ~WorkspaceData() override;
    static WorkspaceData *globalInstance();
public slots:
    void addWorkspace(const QString &dirPath);
    void delWorkspace(const QString &dirPath);
    QStringList findWorkspace(const QString &filePath);
};
#endif // WORKSPACEDATA_H
