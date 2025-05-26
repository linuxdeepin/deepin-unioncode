// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEBUILDERGENERATOR_H
#define CMAKEBUILDERGENERATOR_H

#include "services/builder/buildergenerator.h"
#include "common/project/projectinfo.h"

#include <QObject>

class CMakeBuilderGeneratorPrivate;
class CMakeBuilderGenerator : public dpfservice::BuilderGenerator
{
    Q_OBJECT
    CMakeBuilderGeneratorPrivate *const d;
public:
    explicit CMakeBuilderGenerator();
    virtual ~CMakeBuilderGenerator() override;

    static QString toolKitName() { return "cmake"; }

    BuildCommandInfo getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo) override;
    void appendOutputParser(std::unique_ptr<AbstractOutputParser>& outputParser) override;
    bool checkCommandValidity(const BuildCommandInfo &info, QString &retMsg) override;

signals:

private slots:

private:
};

#endif // CMAKEBUILDERGENERATOR_H
