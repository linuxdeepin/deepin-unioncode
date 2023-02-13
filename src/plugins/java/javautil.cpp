/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "javautil.h"

#include <QProcess>
#include <QDebug>

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
