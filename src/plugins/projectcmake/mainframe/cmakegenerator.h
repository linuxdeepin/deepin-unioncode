#ifndef CMAKEGENERATOR_H
#define CMAKEGENERATOR_H

#include <QObject>

#include "services/project/projectservice.h"

class CMakeGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    QProcess process;
public:
    explicit CMakeGenerator();
    virtual QStandardItem *createRootItem(const QString &projectPath) override;
    virtual QMenu* createIndexMenu(const QModelIndex &index) override;
private slots:
    void processReadAll();
    void processFinished(int code, QProcess::ExitStatus status);
};

#endif // CMAKEGENERATOR_H
