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
#include "commandparser.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QDir>

#include <iostream>

CommandParser &CommandParser::instance()
{
    static CommandParser ins;
    return ins;
}

bool CommandParser::isSet(const QString &name) const
{
    return commandParser->isSet(name);
}

QString CommandParser::value(const QString &name) const
{
    return commandParser->value(name);
}

void CommandParser::process()
{
    return process(qApp->arguments());
}

void CommandParser::process(const QStringList &arguments)
{
    qDebug() << "App start args: " << arguments;
    commandParser->process(arguments);
}

void CommandParser::setModel(CommandParser::CommandModel model)
{
    commandModel = model;
}

CommandParser::CommandModel CommandParser::getModel()
{
    return commandModel;
}

void CommandParser::initialize()
{
    commandParser->setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    initOptions();
    commandParser->addHelpOption();
    commandParser->addVersionOption();
}

void CommandParser::initOptions()
{
    QCommandLineOption buildOption(QStringList()
                                   << "b"
                                   << "build",
                                   "build with deepin-unioncode(won't work with empty directory)",
                                   "directory");
    QCommandLineOption kitOption(QStringList()
                                 << "k"
                                 << "kit {CMake,Gradle,Maven,Ninja}",
                                 "select build kit to build project.Support cmake,gradle,maven,ninja.It is CMake by default.",
                                 "name", "CMake");
    QCommandLineOption argsOption(QStringList()
                                 << "a"
                                 << "arguments",
                                 "input argument to use kit to build project",
                                 "argument list");
    QCommandLineOption newWindowOption(QStringList()
                                       << "n"
                                       << "new-window",
                                       "show new window");
    addOption(buildOption);
    addOption(kitOption);
    addOption(argsOption);
    addOption(newWindowOption);
}

void CommandParser::addOption(const QCommandLineOption &option)
{
    commandParser->addOption(option);
}

QStringList CommandParser::positionalArguments() const
{
    return commandParser->positionalArguments();
}

QStringList CommandParser::unknownOptionNames() const
{
    return commandParser->unknownOptionNames();
}

CommandParser::CommandParser(QObject *parent)
    : QObject(parent),
      commandParser(new QCommandLineParser)
{
    initialize();
}
