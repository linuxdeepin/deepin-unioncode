// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "llgenerator.h"
#include "common/widget/outputpane.h"
#include "services/window/windowservice.h"

#include <yaml-cpp/yaml.h>

using namespace dpfservice;

struct PackageInfo {
    QString id;
    QString name;
};

PackageInfo readPackageFromYaml(const QString &filePath)
{
    YAML::Node config = YAML::LoadFile(filePath.toStdString());

    PackageInfo ret;
    if (config["package"]) {
        YAML::Node package = config["package"];

        if (package["id"]) {
            QString id = QString::fromStdString(package["id"].as<std::string>());
            ret.id = id;
        }

        if (package["name"]) {
            QString name = QString::fromStdString(package["name"].as<std::string>());
            ret.name = name;
        }
    }
    return ret;
}

int getValidPort()
{
    auto checkPortFree = [](int port) {
        QProcess process;
        QString cmd = QString("fuser %1/tcp").arg(port);
        process.start(cmd);
        process.waitForFinished();
        QString ret = process.readAll();
        if (ret.isEmpty())
            return true;
        return false;
    };

    int port = 12345;
    int kPort = port;
    int maxTryNum = 100;

    while (port - kPort < maxTryNum) {
        if (checkPortFree(port)) {
            return port;
        }
        port++;
    }

    return 0;
}

LLGenerator::LLGenerator()
{
    terminalSrv = dpfGetService(TerminalService);
    debuggerSrv = dpfGetService(DebuggerService);
}

LLGenerator::~LLGenerator()
{
}

QString LLGenerator::debugger()
{
    return "";
}

bool LLGenerator::prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg)
{
    Q_UNUSED(param)
    Q_UNUSED(retMsg)

    return true;
}

bool LLGenerator::requestDAPPort(const QString &ppid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    auto workspacePath = param.value("workspace").toString();
    auto executableDir = workspacePath + "/linglong/output/binary/files/bin/";
    if (!QFileInfo(executableDir).exists()) {
        retMsg = tr("This Linglong Project has no executable program");
        return false;
    }

    QMetaObject::invokeMethod(this, [=, workspacePath](){
        auto yamlFile = workspacePath + "/linglong.yaml";
        auto packageInfo = readPackageFromYaml(yamlFile);

        QUuid newConsole = terminalSrv->createConsole("LLDebug", true);
        QProcess startEnv;
        startEnv.setProgram("ll-builder");
        startEnv.setWorkingDirectory(param.value("workspace").toString());
        startEnv.setArguments({ "run", "--debug", "--exec", "/bin/bash" });
        terminalSrv->run2Console(newConsole, startEnv);

        QProcess gdbserver;
        gdbserver.setProgram("gdbserver");
        QString port = QString::number(getValidPort());
        QString binPath = QString("/opt/apps/%1/files/bin/%2").arg(packageInfo.id, packageInfo.name);
        gdbserver.setArguments({ QString(":%1").arg(port),  binPath});
        terminalSrv->run2Console(newConsole, gdbserver);

        GDBServerPreParam preParam;
        preParam.ip = "127.0.0.1";
        preParam.port = port.toInt();
        preParam.projectPath = workspacePath;
        preParam.initCommands.append(QString("set substitute-path /project %1").arg(workspacePath));
        QString debugFilePath = workspacePath + "/linglong/output/develop/files/lib/debug";
        if (QFileInfo(debugFilePath).exists())
            preParam.debugInfo = debugFilePath;
        QString executablePath = workspacePath + "/linglong/output/binary/files/bin/" + packageInfo.name;
        if (QFileInfo(executablePath).exists())
            preParam.executablePath = executablePath;
        debuggerSrv->requestConnectToGdbServer(preParam);
    });

    return false;
}

bool LLGenerator::isNeedBuild()
{
    return false;
}

bool LLGenerator::isTargetReady()
{
    return true;
}

bool LLGenerator::isLaunchNotAttach()
{
    return true;
}

dap::LaunchRequest LLGenerator::launchDAP(const QMap<QString, QVariant> &param)
{
    dap::LaunchRequest request;
    return request;
}

QString LLGenerator::build(const QString &projectPath)
{
    Q_UNUSED(projectPath)
    return "";
}

QString LLGenerator::getProjectFile(const QString &projectPath)
{
    Q_UNUSED(projectPath)
    return "";
}

QMap<QString, QVariant> LLGenerator::getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                       const QString &currentFile)
{
    Q_UNUSED(currentFile)

    QMap<QString, QVariant> param;
    param.insert("workspace", projectInfo.workspaceFolder());

    return param;
}

RunCommandInfo LLGenerator::getRunArguments(const ProjectInfo &projectInfo, const QString &currentFile)
{
    Q_UNUSED(currentFile)
    RunCommandInfo info;
    info.program = "ll-builder";
    info.arguments.append("run");
    info.workingDir = projectInfo.workspaceFolder();
    info.runInTerminal = false;

    return info;
}
