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
    QString sourcePath = CommandParser::instance().value("b");
    QString destPath = CommandParser::instance().value("d");
    QString kit = CommandParser::instance().value("k");
    QStringList args = CommandParser::instance().value("a").trimmed().split(" ", QString::SkipEmptyParts);
    QString elfPath = CommandParser::instance().value("t");

    if (sourcePath.isEmpty()) {
        std::cout << std::string("Please input project directory!") << std::endl;
        return;
    }
    if (!QDir::isAbsolutePath(sourcePath)) {
        QString path = QDir::currentPath();
        sourcePath = path + QDir::separator() + sourcePath;
    }
    if (!QDir(sourcePath).exists()) {
        std::cout << std::string("Please input correct working directory!") << std::endl;
        return;
    }

    QStringList arguments;
    arguments << "-S";
    arguments << sourcePath;
    arguments << "-B";
    arguments << destPath;
    auto &ctx = dpfInstance.serviceContext();
    dpfservice::BuilderService *builderService = ctx.service<dpfservice::BuilderService>(dpfservice::BuilderService::name());

    QList<BuildCommandInfo> buildCommandInfos;
    BuildCommandInfo buildInfo;
    buildInfo.workingDir = sourcePath;
    buildInfo.program = OptionManager::getInstance()->getToolPath(kit);
    buildInfo.kitName = kit.toLower();
    if (buildInfo.kitName == "cmake")
        buildInfo.arguments = arguments;
    buildCommandInfos.append(buildInfo);
    if (!destPath.isEmpty())
        buildInfo.workingDir = destPath;
    buildInfo.arguments = args;
    buildCommandInfos.append(buildInfo);
    QFuture<void> future = QtConcurrent::run([=](){
        builderService->interface.builderCommand(buildCommandInfos, true);
    });
    future.waitForFinished();

    if (CommandParser::instance().isSet("t")) {
        buildInfo.elfPath = elfPath;
        buildInfo.program = "/usr/bin/objcopy";
        QStringList exeFiles = exeFileName(buildInfo.workingDir);
        for (QString exeFile : exeFiles) {
            buildInfo.arguments = QStringList() << "--add-section" << ".note.builder=" + elfPath << exeFile;
        }
    }
    builderService->interface.builderCommand({buildInfo}, true);
}

CommandExecuter::CommandExecuter(QObject *parent)
    :QObject (parent)
{
}

QStringList CommandExecuter::exeFileName(QDir dir)
{
    QStringList exeFiles;
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList files = dir.entryInfoList();
    foreach (QFileInfo fileInfo, files) {
        if (fileInfo.isHidden())
            continue;
        if (fileInfo.isExecutable())
            exeFiles.append(fileInfo.fileName());
    }
    return exeFiles;
}
