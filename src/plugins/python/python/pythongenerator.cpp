// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pythongenerator.h"
#include "python/pythondebug.h"

using namespace dpfservice;

class PythonGeneratorPrivate
{
    friend class PythonGenerator;
    QSharedPointer<PythonDebug> pythonDebug;
};

PythonGenerator::PythonGenerator()
    : d(new PythonGeneratorPrivate())
{
    d->pythonDebug.reset(new PythonDebug());
}

PythonGenerator::~PythonGenerator()
{
    if (d)
        delete d;
}

QString PythonGenerator::debugger()
{
    return "dap";
}

bool PythonGenerator::isTargetReady()
{
    return true;
}

bool PythonGenerator::prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString currentFile = param.value("currentFile").toString();
    return d->pythonDebug->prepareDebug(currentFile, retMsg);
}

bool PythonGenerator::requestDAPPort(const QString &uuid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString fileName = param.value("currentFile").toString();
    QString projectPath = param.value("workspace").toString();
    return d->pythonDebug->requestDAPPort(uuid, toolKitName(), projectPath, fileName, retMsg);
}

bool PythonGenerator::isLaunchNotAttach()
{
    return d->pythonDebug->isLaunchNotAttach();
}

dap::AttachRequest PythonGenerator::attachDAP(int port, const QMap<QString, QVariant> &param)
{
    QString workspace = param.value("workspace").toString();
    return d->pythonDebug->attachDAP(port, workspace);
}

bool PythonGenerator::isRestartDAPManually()
{
    return d->pythonDebug->isRestartDAPManually();
}

bool PythonGenerator::isStopDAPManually()
{
    return d->pythonDebug->isStopDAPManually();
}

QMap<QString, QVariant> PythonGenerator::getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                           const QString &currentFile)
{
    Q_UNUSED(currentFile)

    QMap<QString, QVariant> param;
    param.insert("workspace", projectInfo.workspaceFolder());
    param.insert("currentFile", currentFile);

    return param;
}

static QString getEntryFilePath(const QDir &dir)
{
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    //Create a QRegExp object with the given regular expression
    QRegExp regExp("if\\s*__name__\\s*==\\s*'__main__'");

    //Loop through files
    foreach (QFileInfo entry, entries) {
        if (entry.isDir()) {
            //If the entry is a folder, call the browseRecursively function again
            QDir dir(entry.filePath());
            QString mainClass = getEntryFilePath(dir);
            if (!mainClass.isEmpty())
                return mainClass;
        } else {
            if (entry.suffix().toLower() == "py") {
                QFile file(entry.filePath());
                if (file.open(QIODevice::ReadOnly)) {
                    QString fileContent = file.readAll();
                    file.close();
                    //Check if the given regular expression matches the file content
                    if (regExp.indexIn(fileContent) >= 0) {
                        return entry.filePath();
                    }
                }
            }
        }
    }
    return {};
}

RunCommandInfo PythonGenerator::getRunArguments(const ProjectInfo &projectInfo, const QString &currentFile)
{
    // TODO(Any): put this flag to option config UI.
    bool isRunCurrentFile = false;

    RunCommandInfo runCommandInfo;
    runCommandInfo.program = "python";
    runCommandInfo.arguments << (isRunCurrentFile ? currentFile : getEntryFilePath(projectInfo.workspaceFolder()));
    runCommandInfo.workingDir = projectInfo.workspaceFolder();
    return runCommandInfo;
}
