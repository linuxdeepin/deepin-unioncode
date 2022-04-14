#ifndef CMAKEGENERATOR_H
#define CMAKEGENERATOR_H

#include "services/project/projectservice.h"

#include <QObject>
#include <QDomDocument>

class CMakeGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    QProcess process;
public:
    explicit CMakeGenerator();
    virtual bool configure(const QString &projectPath) override;
    virtual QStandardItem *createRootItem(const QString &projectPath) override;
    virtual QMenu* createIndexMenu(const QModelIndex &index) override;
private slots:
    void processReadAll();
    void processFinished(int code, QProcess::ExitStatus status);
private:
    QStandardItem *cmakeCDT4FindItem(QStandardItem *rootItem, QString &name);
    QStandardItem *cmakeCDT4FindParentItem(QStandardItem *rootItem, QString &name);
    QHash<QString, QString> cmakeCDT4Subporjects(QStandardItem *rootItem);
    QStandardItem *cmakeCDT4DisplayOptimize(QStandardItem *rootItem);
    void cmakeCDT4TargetsDisplayOptimize(QStandardItem *item, const QHash<QString, QString> &subprojectsMap);
    QDomDocument loadXmlDoc(const QString &cmakePath);
    QString cmakeCDT4FilePath(const QString &cmakePath);
    QString cmakeBuildPath(const QString &cmakePath);
};

#endif // CMAKEGENERATOR_H
