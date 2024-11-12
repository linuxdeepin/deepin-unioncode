// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmakebuildergenerator.h"
#include "cmake/builder/parser/ansifilterparser.h"
#include "cmake/builder/parser/gnumakeparser.h"
#include "cmake/builder/parser/gccparser.h"
#include "cmake/builder/parser/cmakeparser.h"
#include "cmake/option/kitmanager.h"

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
        info.arguments = projectInfo.cleanCustomArgs();
    }

    info.program = projectInfo.buildProgram();
    if (info.program.isEmpty()) {
        info.program = KitManager::instance()->defaultKit().cmakeTool().path;
    }

    info.workingDir = projectInfo.buildFolder();
    info.kitName = projectInfo.kitName();

    return info;
}

void CMakeBuilderGenerator::appendOutputParser(std::unique_ptr<AbstractOutputParser> &outputParser)
{
    if (outputParser) {
        outputParser->takeOutputParserChain();
        outputParser->appendOutputParser(new AnsiFilterParser());
        // After appendOutputParser, OutputParser is in a recursive relationship, and the sequence should be cmake -> make -> gcc.
        outputParser->appendOutputParser(new CMakeParser());
        outputParser->appendOutputParser(new GnuMakeParser());
        outputParser->appendOutputParser(new GccParser());
    }
}

bool CMakeBuilderGenerator::checkCommandValidity(const BuildCommandInfo &info, QString &retMsg)
{
    if (info.program.trimmed().isEmpty()) {
        retMsg = tr("The build command %1 project is null! You can solve this problem in the following ways:\n"
                    "1.Check whether cmake is installed;\n"
                    "2.Global Options > CMake > Select the CMake tool installed locally;\n"
                    "3.If none of the above methods work, delete the \".unioncode\" folder in the current project directory and open the project again.")
                         .arg(info.kitName.toUpper());
        return false;
    }

    if (!QFileInfo(info.workingDir.trimmed()).exists()) {
        retMsg = tr("The path of \"%1\" is not exist! "
                    "please check and reopen the project.")
                         .arg(info.workingDir);
        return false;
    }

    return true;
}
