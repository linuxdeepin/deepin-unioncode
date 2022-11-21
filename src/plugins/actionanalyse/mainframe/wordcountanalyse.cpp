/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "wordcountanalyse.h"
#include "common/common.h"

namespace {
const QString wordcount{"wordcount"};
}
WordCountAnalyse::WordCountAnalyse(QObject *parent)
    : QProcess (parent)
{
    auto procEnv = Environment::get(Environment::User, Environment::Python, 3);
    for (auto val : procEnv.keys()) {
        qInfo()<< val << procEnv.value(val);
    }
    setProcessEnvironment(procEnv);
    auto env = processEnvironment().systemEnvironment();
    for (auto val : env.keys()) {
        qInfo() << val << env.value(val);
    }
    QObject::connect(this, &QProcess::errorOccurred,
                     this, &WordCountAnalyse::errorOccurred);
    QObject::connect(this, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     this, &WordCountAnalyse::finished);
//    QObject::connect(this, &QProcess::readAllStandardOutput,
//                     this, &QDebug::redirectErr);
//    QObject::connect(this, &QProcess::readAllStandardError,
//                     this, &WordCountAnalyse::redirectErr);
    setWorkingDirectory(CustomPaths::global(CustomPaths::Scripts)+ QDir::separator() + "action-analysis");
}

void WordCountAnalyse::setArgs(const ActionAnalyseArgs &args)
{
    this->processArgs = args;
}

ActionAnalyseArgs WordCountAnalyse::args() const
{
    return this->processArgs;
}

void WordCountAnalyse::setStorage(const QString &storage)
{
    processArgs.storage = storage;
}

QString WordCountAnalyse::getStorage() const
{
    return processArgs.storage;
}

void WordCountAnalyse::setWorkspace(const QString &workspace)
{
    processArgs.workspace = workspace;
}

QString WordCountAnalyse::getWorkspace() const
{
    return processArgs.workspace;
}

void WordCountAnalyse::setLanguage(const QString &language)
{
    processArgs.language = language;
}

QString WordCountAnalyse::getLanguage() const
{
    return processArgs.language;
}

void WordCountAnalyse::start()
{
    qInfo() << workingDirectory();
    setProcessChannelMode(QProcess::ForwardedChannels);
    setProgram(getPythonVersion());
    QStringList args;
    args << "./main.py";
    args << "-w" << processArgs.workspace;
    args << "-s" << processArgs.storage;
    args << "-l" << processArgs.language;
    setArguments(args);
    QProcess::start();
}

QString WordCountAnalyse::getPythonVersion()
{
    if (pythonVersion.isEmpty()) {
        QDir dir("/usr/bin");
        QStringList filter {"Python*.*"};
        QStringList pythonList = dir.entryList(filter);

        QRegExp reg("((\\d)|(\\d.\\d))($|\\s)");
        reg.setMinimal(true);
        int position = 0;
        QList<QString> versions;
        while (position >= 0) {
            position = reg.indexIn(pythonList.join(" "), position);
            if (position < 0)
                break;
            versions << reg.cap(1);
            position += reg.matchedLength();
        }

        double newVersion = 0;
        for (auto version : versions) {
            double v = version.toDouble();
            if (v > newVersion) {
                newVersion = v;
            }
        }
        pythonVersion = "python" + QString::number(newVersion);
    }
    return  pythonVersion;
}

ActionAnalyseArgs::ActionAnalyseArgs()
{

}

ActionAnalyseArgs::ActionAnalyseArgs(const QString &workspace,
                                 const QString &language,
                                 const QString &storage)
    : workspace(workspace)
    , language(language)
    , storage(storage)
{

}

ActionAnalyseArgs::ActionAnalyseArgs(const ActionAnalyseArgs &as)
    : workspace(as.workspace)
    , language(as.language)
    , storage(as.storage)
{

}

ActionAnalyseArgs &ActionAnalyseArgs::operator=(const ActionAnalyseArgs &as)
{
    workspace = as.workspace;
    language = as.language;
    storage = as.storage;
    return *this;
}

void WordCountAnalyse::errorOccurred(QProcess::ProcessError err)
{
    qCritical() << exitCode() << exitStatus() << err << readAllStandardError();
}

void WordCountAnalyse::finished(int exitCode, QProcess::ExitStatus status)
{
    qCritical() << exitCode << status << readAllStandardOutput();
    if (exitCode == 0)
        analyseDone(true);
    else
        analyseDone(false);
}
