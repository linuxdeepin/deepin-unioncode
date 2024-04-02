// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "symbolparser.h"
#include "common/common.h"
#include "clangparser/clangparser.h"

namespace {
const QString unionparser{"unionparser"};
}
SymbolParser::SymbolParser(QObject *parent)
    : QProcess (parent)
{
    setProgram(getPython());
    auto procEnv = env::lang::get(env::lang::User, env::lang::Python, 3);
    for (auto val : procEnv.keys()) {
        qInfo()<< val << procEnv.value(val);
    }
    setProcessEnvironment(procEnv);
    auto env = processEnvironment().systemEnvironment();
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
#ifdef __mips__
    // switch to libclang.
    ClangParser::parse(processArgs.workspace, processArgs.storage, processArgs.language);
#else
    QString scriptPath = CustomPaths::global(CustomPaths::Scripts);
    QStringList arguments;
    arguments << (scriptPath + "/symbol-parser/main.py");
    arguments << "-w" << processArgs.workspace;
    arguments << "-l" << processArgs.language;
    arguments << "-s" << processArgs.storage;
    setArguments({arguments});
    QProcess::start();
#endif
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
}

void SymbolParser::redirectErr()
{
    qCritical() << "symbol parser error:" <<QProcess::readAllStandardError();
}

QList<QString> findAll(const QString &pattern, const QString &str, bool greedy)
{
    QRegExp rxlen(pattern);
    rxlen.setMinimal(greedy);
    int position = 0;
    QList<QString> strList;
    while (position >= 0) {
        position = rxlen.indexIn(str, position);
        if (position < 0)
            break;
        strList << rxlen.cap(1);
        position += rxlen.matchedLength();
    }
    return strList;
}

QString SymbolParser::getPython()
{
    if (pythonCmd.isEmpty()) {
        QDir dir("/usr/bin");
        QStringList filter { "Python*.*" };
        dir.setNameFilters(filter);
        QStringList pythonList = dir.entryList();

        QString pattern = "((\\d)|(\\d+.\\d+))($|\\s)";
        QStringList versions = findAll(pattern, pythonList.join(" "), true);

        double newVersion = 0;
        for (auto version : versions) {
            double v = version.toDouble();
            if (v > newVersion) {
                newVersion = v;
            }
        }
        pythonCmd = "python" + QString::number(newVersion);
    }
    return pythonCmd;
}
