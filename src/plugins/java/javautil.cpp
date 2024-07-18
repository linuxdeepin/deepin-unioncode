// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "javautil.h"
#include "services/window/windowservice.h"

#include <QProcess>
#include <QDebug>
#include <QStandardPaths>

QString JavaUtil::getMainClassPath(const QDir &dir)
{
    QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    //Create a QRegExp object with the given regular expression
    QRegExp regExp("public\\s+static\\s+void\\s+main\\s*\\(\\s*");

    //Loop through files
    foreach (QFileInfo entry, entries) {
        if (entry.isDir()) {
            //If the entry is a folder, call the browseRecursively function again
            QDir dir(entry.filePath());
            QString mainClass = getMainClassPath(dir);
            if (!mainClass.isEmpty())
                return mainClass;
        } else {
            //If the file is not a folder, then check if it has the .class extension
            if (entry.suffix().toLower() == "class") {
                qInfo() << entry.fileName();

                if (QStandardPaths::findExecutable("javap").isEmpty()) {
                    auto winSrv = dpfGetService(dpfservice::WindowService);
                    winSrv->notify(2, "Java", tr("Unable to find the javap application. Please install openjdk-11-jdk or another version first."), {});
                    return {};
                }

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

QString JavaUtil::getMainClass(const QString &mainClassPath, const QString &packageDirName)
{
    QString mainClass;
    if (!mainClassPath.isEmpty()) {
        int index = mainClassPath.indexOf(packageDirName);
        mainClass = mainClassPath.mid(index + packageDirName.size() + 1);
        mainClass.remove(".class");
        mainClass.replace("/", ".");
    }
    return mainClass;
}

QString JavaUtil::getPackageDir(const QString &mainClassPath, const QString &packageDirName)
{
    QString packageDir;
    if (!mainClassPath.isEmpty()) {
        QString targetPath = mainClassPath.left(mainClassPath.indexOf(packageDirName));
        packageDir = targetPath + packageDirName;
    }
    return packageDir;
}
