// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ninjabuildergenerator.h"
#include "ninja/builder/parser/ninjaparser.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

using namespace dpfservice;

class NinjaBuilderGeneratorPrivate
{
    friend class NinjaBuilderGenerator;
};

NinjaBuilderGenerator::NinjaBuilderGenerator()
    : d(new NinjaBuilderGeneratorPrivate())
{

}

NinjaBuilderGenerator::~NinjaBuilderGenerator()
{
    if (d)
        delete d;
}

BuildCommandInfo NinjaBuilderGenerator::getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo)
{
    BuildCommandInfo info;
    info.kitName = projectInfo.kitName();
    info.workingDir = projectInfo.workspaceFolder();
    info.program = projectInfo.buildProgram();
    if (info.program.isEmpty())
        info.program = OptionManager::getInstance()->getNinjaToolPath();
    switch (buildMenuType) {
    case Build:
        info.arguments.append("all");
        break;
    case Clean:
        info.arguments.append("clean");
        break;
    }

    return info;
}

void NinjaBuilderGenerator::appendOutputParser(std::unique_ptr<IOutputParser> &outputParser)
{
    if (outputParser) {
        outputParser->takeOutputParserChain();
        outputParser->appendOutputParser(new NinjaParser());
    }
}


bool NinjaBuilderGenerator::checkCommandValidity(const BuildCommandInfo &info, QString &retMsg)
{
    if (info.program.trimmed().isEmpty()) {
        retMsg = tr("The build command of %1 project is null! "\
                    "please install it in console with \"sudo apt install ninja-build\", and then restart the tool.")
                .arg(info.kitName.toUpper());
        return false;
    }

    if (!QFileInfo(info.workingDir.trimmed()).exists()) {
        retMsg = tr("The path of \"%1\" is not exist! "\
                    "please check and reopen the project.")
                .arg(info.workingDir);
        return false;
    }

    return true;
}

