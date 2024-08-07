// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LLPROJECTGENERATOR_H
#define LLPROJECTGENERATOR_H

#include "llglobal.h"
#include "services/project/projectservice.h"
#include "services/project/directorygenerator.h"

class LLProjectGenerator : public dpfservice::DirectoryGenerator
{
    Q_OBJECT
public:
    explicit LLProjectGenerator();
    virtual ~LLProjectGenerator();

    static QString toolKitName() { return LL_TOOLKIT; }
    virtual QString configureKitName() override { return LL_TOOLKIT; }
    virtual QStringList supportLanguages() override;
    virtual QStringList supportFileNames() override { return {"linglong.yaml"}; }
    virtual DWidget *configureWidget(const QString &language,
                                     const QString &projectPath) override;
    virtual void acceptConfigure() override;
    virtual QMenu *createItemMenu(const QStandardItem *item) override;

private slots:
    void actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item);
};

#endif   // LLPROJECTGENERATOR_H
