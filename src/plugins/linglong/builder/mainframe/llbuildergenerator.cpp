// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "llbuildergenerator.h"
#include "builder/parser/llparser.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

using namespace dpfservice;

LLBuilderGenerator::LLBuilderGenerator()
{
}

LLBuilderGenerator::~LLBuilderGenerator()
{
}

BuildCommandInfo LLBuilderGenerator::getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo)
{
    Q_UNUSED(buildMenuType)
    BuildCommandInfo info;
    info.kitName = projectInfo.kitName();
    info.workingDir = projectInfo.workspaceFolder();
    info.program = "ll-builder";
    info.arguments.append("build");

    return info;
}

void LLBuilderGenerator::appendOutputParser(std::unique_ptr<AbstractOutputParser> &outputParser)
{
    if (outputParser) {
        outputParser->takeOutputParserChain();
        outputParser->appendOutputParser(new LLParser());
    }
}

bool LLBuilderGenerator::checkCommandValidity(const BuildCommandInfo &info, QString &retMsg)
{
    if (!QFileInfo(info.workingDir.trimmed()).exists()) {
        retMsg = tr("The path of \"%1\" is not exist! "
                    "please check and reopen the project.")
                         .arg(info.workingDir);
        return false;
    }

    return true;
}
