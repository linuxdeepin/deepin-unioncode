// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAVENPROJECTGENERATOR_H
#define MAVENPROJECTGENERATOR_H

#include "mavenasynparse.h"
#include "services/project/projectservice.h"

#include <QObject>
#include <QDomDocument>

class MavenProjectGeneratorPrivate;
namespace config {
class ProjectConfigure;
}
class MavenProjectGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    MavenProjectGeneratorPrivate *const d;
public:
    explicit MavenProjectGenerator();
    virtual ~MavenProjectGenerator();
    static QString toolKitName() { return "maven"; }
    virtual QStringList supportLanguages() override;
    virtual QStringList supportFileNames() override;
    virtual DWidget* configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual void acceptConfigure() override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem* root) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;
private slots:
    void itemModified(const QList<QStandardItem*> &item);
    void doAddMavenMeue(const ProjectActionInfos &infos);
    void doActionTriggered();
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);

private:
    void restoreRuntimeCfg(dpfservice::ProjectInfo &info);
};

#endif // MAVENPROJECTGENERATOR_H
