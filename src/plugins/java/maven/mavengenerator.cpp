/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "mavengenerator.h"

#include "java/javadebug.h"
#include "maven/mavenbuild.h"

#include <QRegExp>

using namespace dpfservice;

class MavenGeneratorPrivate
{
    friend class MavenGenerator;
    QSharedPointer<JavaDebug> javaDebug;
};

MavenGenerator::MavenGenerator()
    : d(new MavenGeneratorPrivate())
{
    d->javaDebug.reset(new JavaDebug());
}

MavenGenerator::~MavenGenerator()
{
    if (d)
        delete d;
}

bool MavenGenerator::isNeedBuild()
{
    return true;
}

bool MavenGenerator::isTargetReady()
{
    return true;
}

bool MavenGenerator::isAnsyPrepareDebug()
{
    return d->javaDebug->isAnsyPrepareDebug();
}

bool MavenGenerator::prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString projectPath = param.value("workspace").toString();
    return d->javaDebug->prepareDebug(projectPath, retMsg);
}

bool MavenGenerator::requestDAPPort(const QString &uuid, const QMap<QString, QVariant> &param, QString &retMsg)
{
    QString projectPath = param.value("workspace").toString();
    return d->javaDebug->requestDAPPort(uuid, toolKitName(), projectPath, retMsg);
}

bool MavenGenerator::isLaunchNotAttach()
{
    return d->javaDebug->isLaunchNotAttach();
}

dap::LaunchRequest MavenGenerator::launchDAP(const QMap<QString, QVariant> &param)
{
    QString workspace = param.value("workspace").toString();
    QString mainClass = param.value("mainClass").toString();
    QString projectName = param.value("projectName").toString();
    QStringList classPaths = param.value("classPaths").toStringList();

    return d->javaDebug->launchDAP(workspace, mainClass, projectName, classPaths);
}

bool MavenGenerator::isRestartDAPManually()
{
    return d->javaDebug->isRestartDAPManually();
}

bool MavenGenerator::isStopDAPManually()
{
    return d->javaDebug->isStopDAPManually();
}

QString MavenGenerator::build(const QString &projectPath)
{
    return MavenBuild::build(toolKitName(), projectPath);
}

QString MavenGenerator::getProjectFile(const QString &projectPath)
{
    return projectPath + QDir::separator() + "pom.xml";
}

QMap<QString, QVariant> MavenGenerator::getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                                          const QString &currentFile)
{
    Q_UNUSED(currentFile)

    QMap<QString, QVariant> param;
    param.insert("workspace", projectInfo.workspaceFolder());

    return param;
}

QString getMainClass(const QDir &dir)
{
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    //Create a QRegExp object with the given regular expression
    QRegExp regExp("public\\s+static\\s+void\\s+main\\s*\\(\\s*");

    //Loop through files
    foreach (QFileInfo entry, entries) {
        if (entry.isDir()) {
            //If the entry is a folder, call the browseRecursively function again
            QDir dir(entry.filePath());
            QString mainClass = getMainClass(dir);
            if (!mainClass.isEmpty())
                return mainClass;
        } else {
            //If the file is not a folder, then check if it has the .class extension
            if (entry.suffix().toLower() == "class") {
                qInfo() << entry.fileName();

                QProcess process;
                auto temp = entry.filePath();
                process.start("javap " + entry.filePath());
                if(!process.waitForFinished()) {
                    qDebug() << "process is error!";
                    break;
                }
                QString output = process.readAllStandardOutput();
                //Check if the given regular expression matches the file content
                if (regExp.indexIn(output) >= 0) {
                    return entry.filePath();
                }
            }
        }
    }
    return {};
}

RunCommandInfo MavenGenerator::getRunArguments(const ProjectInfo &projectInfo, const QString &currentFile)
{
    Q_UNUSED(currentFile)

    RunCommandInfo runCommandInfo;

    QString mainClassPath = getMainClass(projectInfo.workspaceFolder());
    if (!mainClassPath.isEmpty()) {
        QString classes = "classes";
        QString targetPath = mainClassPath.left(mainClassPath.indexOf(classes));
        QString classesPath = targetPath + classes;

        int index = mainClassPath.indexOf(classes);
        QString subStrings = mainClassPath.mid(index + classes.size() + 1);
        subStrings.remove(".class");
        subStrings.replace("/", ".");

        runCommandInfo.program = "java";
        runCommandInfo.arguments << subStrings;
        runCommandInfo.workingDir = classesPath;
    }

    return runCommandInfo;
}
