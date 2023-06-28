// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    if (!QDir::isAbsolutePath(destPath))
        destPath = QDir::currentPath() + QDir::separator() + destPath;
    if (!QDir::isAbsolutePath(elfPath))
        elfPath = QDir::currentPath() + QDir::separator() + elfPath;

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
