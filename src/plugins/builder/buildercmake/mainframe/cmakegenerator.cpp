/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "cmakegenerator.h"
#include "parser/ansifilterparser.h"
#include "parser/gnumakeparser.h"
#include "parser/gccparser.h"
#include "parser/cmakeparser.h"

#include "services/builder/builderservice.h"
#include "services/builder/ioutputparser.h"
#include "services/project/projectservice.h"
#include "services/option/optionmanager.h"

using namespace dpfservice;

class CMakeGeneratorPrivate
{
    friend class CMakeGenerator;
};

CMakeGenerator::CMakeGenerator()
    : d(new CMakeGeneratorPrivate())
{

}

CMakeGenerator::~CMakeGenerator()
{
    if (d)
        delete d;
}

void CMakeGenerator::getMenuCommand(BuildCommandInfo &info, const BuildMenuType buildMenuType)
{
    info.program = OptionManager::getInstance()->getCMakeToolPath();
    TargetType type = kBuildTarget;
    switch (buildMenuType) {
    case Build:
        type = kBuildTarget;
        break;
    case Clean:
        type = kCleanTarget;
        break;
    }
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && projectService->getActiveTarget) {
        auto target = projectService->getActiveTarget(type);
        if (!target.buildCommand.isEmpty()) {
            QStringList args = target.buildArguments << target.buildTarget;
            info.program = target.buildCommand;
            info.arguments = args;
            info.workingDir = target.outputPath;
        }
    }
}

void CMakeGenerator::appendOutputParser(std::unique_ptr<IOutputParser>& outputParser)
{
    if (outputParser) {
        outputParser->takeOutputParserChain();
        outputParser->appendOutputParser(new AnsiFilterParser());
        outputParser->appendOutputParser(new GnuMakeParser());
        outputParser->appendOutputParser(new GccParser());
        outputParser->appendOutputParser(new CMakeParser());
    }
}

bool CMakeGenerator::checkCommandValidity(const BuildCommandInfo &info, QString &retMsg)
{
    if (info.program.trimmed().isEmpty()) {
        retMsg = tr("The build command of %1 project is null! "\
                    "please install it in console with \"sudo apt install cmake\", and then restart the tool.")
                .arg(info.kitName.toUpper());

        return false;
    }

    return true;
}

