/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
#include "commandexecuter.h"
#include "common/util/commandparser.h"

#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QDebug>
#include <QDir>

#include <iostream>

CommandExecuter &CommandExecuter::instance()
{
    static CommandExecuter ins;
    return ins;
}

void CommandExecuter::buildProject()
{
    QString projectPath = CommandParser::instance().value("b");
    QString kit = CommandParser::instance().value("k");
    QStringList args = CommandParser::instance().value("a").trimmed().split("", QString::SkipEmptyParts);
    if (projectPath.isEmpty()) {
        std::cout << std::string("Please input project directory!") << std::endl;
        return;
    }
    if (!QDir::isAbsolutePath(projectPath)) {
        QString path = QDir::currentPath();
        projectPath = path + QDir::separator() + projectPath;
    }
    if (!QDir(projectPath).exists()) {
        std::cout << std::string("Please input correct working directory!") << std::endl;
        return;
    }
    auto &ctx = dpfInstance.serviceContext();
    dpfservice::BuilderService *builderService = ctx.service<dpfservice::BuilderService>(dpfservice::BuilderService::name());
    BuildCommandInfo buildInfo;
    buildInfo.workingDir = projectPath;
    buildInfo.arguments = args;
    buildInfo.program = OptionManager::getInstance()->getToolPath(kit);
    buildInfo.kitName = kit.toLower();
    builderService->interface.builderCommand(buildInfo);
}

CommandExecuter::CommandExecuter(QObject *parent)
    : QObject(parent)
{
}
