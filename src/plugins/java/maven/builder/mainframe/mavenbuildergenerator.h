// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAVENBUILDERGENERATOR_H
#define MAVENBUILDERGENERATOR_H

#include "services/builder/buildergenerator.h"

class MavenBuilderGeneratorPrivate;
class MavenBuilderGenerator : public dpfservice::BuilderGenerator
{
    Q_OBJECT
public:
    explicit MavenBuilderGenerator();
    virtual ~MavenBuilderGenerator() override;

    static QString toolKitName() { return "maven"; }

    BuildCommandInfo getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo) override;
    void appendOutputParser(std::unique_ptr<IOutputParser> &outputParser) override;
    bool checkCommandValidity(const BuildCommandInfo &info, QString &retMsg) override;

signals:

private slots:

private:

    MavenBuilderGeneratorPrivate *const d;
};

#endif // MAVENBUILDERGENERATOR_H
