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
#include "cmakebuildergenerator.h"
#include "cmake/builder/parser/ansifilterparser.h"
#include "cmake/builder/parser/gnumakeparser.h"
#include "cmake/builder/parser/gccparser.h"
#include "cmake/builder/parser/cmakeparser.h"

#include "services/builder/builderservice.h"
#include "services/builder/ioutputparser.h"
#include "services/project/projectservice.h"
#include "services/option/optionmanager.h"

using namespace dpfservice;

class CMakeBuilderGeneratorPrivate
{
    friend class CMakeBuilderGenerator;
};

CMakeBuilderGenerator::CMakeBuilderGenerator()
    : d(new CMakeBuilderGeneratorPrivate())
{

}

CMakeBuilderGenerator::~CMakeBuilderGenerator()
{
    if (d)
        delete d;
}

BuildCommandInfo CMakeBuilderGenerator::getMenuCommand(const BuildMenuType buildMenuType, const ProjectInfo &projectInfo)
{
    BuildCommandInfo info;
    if (buildMenuType == Build) {
        info.arguments = projectInfo.buildCustomArgs();
    } else if (buildMenuType == Clean) {
        info.arguments = projectInfo.cleanCustomArgs();
    }

    info.program = projectInfo.buildProgram();
    if (info.program.isEmpty()) {
        info.program = OptionManager::getInstance()->getCMakeToolPath();
    }

    info.workingDir = projectInfo.buildFolder();
    info.kitName = projectInfo.kitName();

    return info;
}

void CMakeBuilderGenerator::appendOutputParser(std::unique_ptr<IOutputParser>& outputParser)
{
    if (outputParser) {
        outputParser->takeOutputParserChain();
        outputParser->appendOutputParser(new AnsiFilterParser());
        outputParser->appendOutputParser(new GnuMakeParser());
        outputParser->appendOutputParser(new GccParser());
        outputParser->appendOutputParser(new CMakeParser());
    }
}

bool CMakeBuilderGenerator::checkCommandValidity(const BuildCommandInfo &info, QString &retMsg)
{
    if (info.program.trimmed().isEmpty()) {
        retMsg = tr("The build command of %1 project is null! "\
                    "please install it in console with \"sudo apt install cmake\", and then restart the tool.")
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

