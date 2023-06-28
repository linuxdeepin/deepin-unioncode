// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADLEBUILDERGENERATOR_H
#define GRADLEBUILDERGENERATOR_H

#include "services/builder/buildergenerator.h"

class GradleBuilderGeneratorPrivate;
class GradleBuilderGenerator : public dpfservice::BuilderGenerator
{
    Q_OBJECT
public:
    explicit GradleBuilderGenerator();
    virtual ~GradleBuilderGenerator() override;

    static QString toolKitName() { return "gradle"; }

    BuildCommandInfo getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo) override;
    void appendOutputParser(std::unique_ptr<IOutputParser> &outputParser) override;
    bool checkCommandValidity(const BuildCommandInfo &info, QString &retMsg) override;

signals:

private slots:
private:
    GradleBuilderGeneratorPrivate *const d;
};

#endif // GRADLEBUILDERGENERATOR_H
