// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LLBUILDERGENERATOR_H
#define LLBUILDERGENERATOR_H

#include "llglobal.h"
#include "services/builder/buildergenerator.h"

class LLBuilderGenerator : public dpfservice::BuilderGenerator
{
    Q_OBJECT
public:
    explicit LLBuilderGenerator();
    virtual ~LLBuilderGenerator() override;

    static QString toolKitName() { return LL_TOOLKIT; }

    BuildCommandInfo getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo) override;
    void appendOutputParser(std::unique_ptr<AbstractOutputParser> &outputParser) override;
    bool checkCommandValidity(const BuildCommandInfo &info, QString &retMsg) override;
};

#endif   // LLBUILDERGENERATOR_H
