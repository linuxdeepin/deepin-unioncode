/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "symbolparser.h"
#include "common/common.h"

namespace {
const QString unionparser{"unionparser"};
}
SymbolParser::SymbolParser(QObject *parent)
    : QProcess (parent)
{
    setProgram("/usr/bin/bash");
    auto procEnv = env::lang::get(env::lang::User, env::lang::Python, 3);
    for (auto val : procEnv.keys()) {
        qInfo()<< val << procEnv.value(val);
    }
    setProcessEnvironment(procEnv);
    auto env = processEnvironment().systemEnvironment();
    for (auto val : env.keys()) {
        qInfo() << val << env.value(val);
    }
    QObject::connect(this, &QProcess::errorOccurred,
                     this, &SymbolParser::errorOccurred);
    QObject::connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, &SymbolParser::finished);
    QObject::connect(this, &QProcess::readyReadStandardError,
                     this, &SymbolParser::redirectErr);
    QObject::connect(this, &QProcess::readyReadStandardOutput,
                     this, &SymbolParser::redirectOut);
 }

void SymbolParser::setArgs(const SymbolParseArgs &args)
{
    this->processArgs = args;
}

SymbolParseArgs SymbolParser::args() const
{
    return this->processArgs;
}

void SymbolParser::setStorage(const QString &storage)
{
    processArgs.storage = storage;
}

QString SymbolParser::getStorage() const
{
    return processArgs.storage;
}

void SymbolParser::setWorkspace(const QString &workspace)
{
    processArgs.workspace = workspace;
}

QString SymbolParser::getWorkspace() const
{
    return processArgs.workspace;
}

void SymbolParser::setLanguage(const QString &language)
{
    processArgs.language = language;
}

QString SymbolParser::getLanguage() const
{
    return processArgs.language;
}

void SymbolParser::start()
{
#ifdef QT_DEBUG
    setProcessChannelMode(QProcess::ForwardedChannels);
#endif
    QString programLine = "unionparser";
    programLine += " -w " + processArgs.workspace;
    programLine += " -l " + processArgs.language;
    programLine += " -s " + processArgs.storage;
    setArguments({"-c", programLine});
    QProcess::start();
}

SymbolParseArgs::SymbolParseArgs()
{

}

SymbolParseArgs::SymbolParseArgs(const QString &workspace,
                                 const QString &language,
                                 const QString &storage)
    : workspace(workspace)
    , language(language)
    , storage(storage)
{

}

SymbolParseArgs::SymbolParseArgs(const SymbolParseArgs &as)
    : workspace(as.workspace)
    , language(as.language)
    , storage(as.storage)
{

}

SymbolParseArgs &SymbolParseArgs::operator=(const SymbolParseArgs &as)
{
    workspace = as.workspace;
    language = as.language;
    storage = as.storage;
    return *this;
}

void SymbolParser::errorOccurred(QProcess::ProcessError err)
{
    qCritical() << exitCode() << exitStatus() << err;
}

void SymbolParser::finished(int exitCode, QProcess::ExitStatus status)
{
    qCritical() << exitCode << status;
    if (exitCode == 0)
        parseDone(true);
    else
        parseDone(false);
}

void SymbolParser::redirectOut()
{
    qDebug() << QProcess::readAllStandardOutput();
}

void SymbolParser::redirectErr()
{
    qCritical() << QProcess::readAllStandardError();
}
