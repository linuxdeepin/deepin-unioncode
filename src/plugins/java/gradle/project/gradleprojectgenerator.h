// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADLEPROJECTGENERATOR_H
#define GRADLEPROJECTGENERATOR_H

#include "gradleasynparse.h"
#include "services/project/projectservice.h"

#include <QObject>
#include <QDomDocument>

class GradleProjectGeneratorPrivate;
class GradleProjectGenerator : public dpfservice::ProjectGenerator
{
    Q_OBJECT
    GradleProjectGeneratorPrivate *const d;
public:
    explicit GradleProjectGenerator();
    ~GradleProjectGenerator() override;
    static QString toolKitName() { return "gradle"; }
    virtual QStringList supportLanguages() override;
    virtual QStringList supportFileNames() override;
    virtual QDialog* configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual bool configure(const dpfservice::ProjectInfo &info = {}) override;
    virtual QStandardItem *createRootItem(const dpfservice::ProjectInfo &info) override;
    virtual void removeRootItem(QStandardItem* root) override;
    virtual QMenu* createItemMenu(const QStandardItem *item) override;

private slots:
    void doProjectChildsModified(const QList<QStandardItem*> &items);
    void doGradleGeneratMenu(const QString &program, const QStringList &args, const QString &workdir);
    void doGradleCleanMenu();
    void doGradleTaskActionTriggered();
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);

private:
    void restoreRuntimeCfg(dpfservice::ProjectInfo &info);
};

#endif // GRADLEPROJECTGENERATOR_H
