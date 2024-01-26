// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gradlebuildergenerator.h"
#include "gradle/builder/parser/gradleparser.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

using namespace dpfservice;

class GradleBuilderGeneratorPrivate
{
    friend class GradleBuilderGenerator;
};

GradleBuilderGenerator::GradleBuilderGenerator()
    : d(new GradleBuilderGeneratorPrivate())
{

}

GradleBuilderGenerator::~GradleBuilderGenerator()
{
    if (d)
        delete d;
}

BuildCommandInfo GradleBuilderGenerator::getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo)
{
    BuildCommandInfo info;
    info.kitName = projectInfo.kitName();
    info.workingDir = projectInfo.workspaceFolder();
    info.program = projectInfo.buildProgram();
    if (info.program.isEmpty())
        info.program = OptionManager::getInstance()->getGradleToolPath();
    switch (buildMenuType) {
    case Build:
        info.arguments.append("build");
        break;
    case Clean:
        info.arguments.append("clean");
        break;
    }

    QString gradlewPath = info.workingDir + QDir::separator() + "gradlew";
    if (QFileInfo(gradlewPath).isFile()) {
        QFileDevice::Permissions p = QFile(gradlewPath).permissions();
        QFile(gradlewPath).setPermissions(p | QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    }

    return info;
}

void GradleBuilderGenerator::appendOutputParser(std::unique_ptr<AbstractOutputParser> &outputParser)
{
    if (outputParser) {
        outputParser->takeOutputParserChain();
        outputParser->appendOutputParser(new GradleParser());
    }
}


bool GradleBuilderGenerator::checkCommandValidity(const BuildCommandInfo &info, QString &retMsg)
{
    if (info.program.trimmed().isEmpty()) {
        retMsg = tr("The build command of %1 project is null! "\
                    "please install it in console with \"sudo apt install gradle\", and then restart the tool.")
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

