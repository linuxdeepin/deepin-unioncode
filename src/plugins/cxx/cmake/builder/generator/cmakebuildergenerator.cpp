// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakebuildergenerator.h"
#include "cmake/builder/parser/ansifilterparser.h"
#include "cmake/builder/parser/gnumakeparser.h"
#include "cmake/builder/parser/gccparser.h"
#include "cmake/builder/parser/cmakeparser.h"

#include "services/builder/builderservice.h"
#include "base/abstractoutputparser.h"
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
        // TODO(Mozart):cleanCustomArgs not fill correct, modify it later.
//        info.arguments = projectInfo.cleanCustomArgs();
        info.arguments.clear();
        info.arguments << "--build";
        info.arguments << ".";
        info.arguments << "--target";
        info.arguments << "clean";
    }

    info.program = projectInfo.buildProgram();
    if (info.program.isEmpty()) {
        info.program = OptionManager::getInstance()->getCMakeToolPath();
    }

    info.workingDir = projectInfo.buildFolder();
    info.kitName = projectInfo.kitName();

    return info;
}

void CMakeBuilderGenerator::appendOutputParser(std::unique_ptr<AbstractOutputParser>& outputParser)
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

