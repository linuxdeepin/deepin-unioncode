// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NINJABUILDERGENERATOR_H
#define NINJABUILDERGENERATOR_H

#include "services/builder/buildergenerator.h"

class NinjaBuilderGeneratorPrivate;
class NinjaBuilderGenerator : public dpfservice::BuilderGenerator
{
    Q_OBJECT
public:
    explicit NinjaBuilderGenerator();
    virtual ~NinjaBuilderGenerator() override;

    static QString toolKitName() { return "ninja"; }

    BuildCommandInfo getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo) override;
    void appendOutputParser(std::unique_ptr<AbstractOutputParser> &outputParser) override;
    bool checkCommandValidity(const BuildCommandInfo &info, QString &retMsg) override;

signals:

private slots:
private:
    NinjaBuilderGeneratorPrivate *const d;
};

#endif // NINJABUILDERGENERATOR_H
