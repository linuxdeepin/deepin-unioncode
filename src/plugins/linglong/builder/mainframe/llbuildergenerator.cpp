// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DDialog>

#include "llbuildergenerator.h"
#include "builder/parser/llparser.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

using namespace dpfservice;
using DTK_WIDGET_NAMESPACE::DDialog;

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
    QProcess process;
    process.start("which", { "ll-builder" });
    process.waitForFinished();
    if (process.exitCode() != 0) {
        DDialog dialog;
        dialog.setWindowTitle("Warning");
        dialog.setIcon(QIcon::fromTheme("dialog-warning"));
        dialog.setMessage(tr("Can`t find linglong-builder tool, Install it based on %1").arg("https://linglong.dev/guide/start/install.html"));
        dialog.exec();
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
