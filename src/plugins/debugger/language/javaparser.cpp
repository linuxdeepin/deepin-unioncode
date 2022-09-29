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
#include "javaparser.h"

#include <QRegularExpression>
#include <QIODevice>
#include <QFile>
#include <QTextStream>
#include <QDir>

namespace javaparser {

const auto CLASS_PATTERN = QRegularExpression("class\\s+([$_a-zA-Z][$_a-zA-Z0-9]*)\\s*");
const auto PACKAGE_PATTERN = QRegularExpression("package\\s+([a-zA-Z_][a-zA-Z0-9_]*)+([.][a-zA-Z_][a-zA-Z0-9_]*)+\\s*;");
const auto MAINCLASS_PATTERN = QRegExp("public\\s+static\\s+void\\s+main\\s*[(]\\s*String\\s*\\[\\s*\\]\\s+([$_a-zA-Z][$_a-zA-Z0-9]*)\\s*[)]");

QString getPackageName(QString lineText)
{
    QRegularExpressionMatch regMatch;
    if ((regMatch = PACKAGE_PATTERN.match(lineText)).hasMatch()) {
        QString retString = regMatch.captured(1) + regMatch.captured(2);
        return retString.trimmed();
    }
    return "";
}

QString getClassName(QString lineText)
{
    QRegularExpressionMatch regMatch;
    if ((regMatch = CLASS_PATTERN.match(lineText)).hasMatch()) {
        QString retString = regMatch.captured(1);
        return retString.trimmed();
    }
    return "";
}

bool getBreakPointPosInfo(BreakPointPosInfo &bpPosInfo, QString filePath, int pos)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    int count = 0;
    QStringList packageNameList;
    QStringList classNameList;

    QTextStream in(&file);
    QString line = in.readLine();
    while(!line.isNull()) {
        if (count++ == pos)
            break;

        QString packageName = getPackageName(line);
        if (!packageName.trimmed().isEmpty())
            packageNameList.push_back(packageName);

        QString className = getClassName(line);
        if (!className.trimmed().isEmpty())
            classNameList.push_back(className);

        line = in.readLine();
    }

    if (packageNameList.isEmpty() || classNameList.isEmpty())
        return false;

    bpPosInfo.className = packageNameList.last() + "." + classNameList.last();
    bpPosInfo.filePos = pos;

    return true;
}

QString getSourcePath(QString projectRootPath)
{
    return projectRootPath + "/src/main/java";
}

QString getClassesPath(QString projectRootPath, bool maven)
{
    if (maven)
        return projectRootPath + "/target/classes";
    else
        return projectRootPath + "/bin/main";
}

QStringList getAllCustomFiles(QString sourcePath, QString suffix)
{
    QStringList filePathList;

    QDir dir(sourcePath);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files
                                                   | QDir::NoDotAndDotDot
                                                   | QDir::Dirs);
    foreach (auto fileInfo, fileInfoList) {
        if(fileInfo.isDir()){
            QString fileDir = fileInfo.absoluteFilePath();
            filePathList.append(getAllCustomFiles(fileDir, suffix));
        } else {
            if (suffix.toUpper() == fileInfo.completeSuffix().toUpper()) {
                filePathList.append(fileInfo.absoluteFilePath());
            }
        }
    }

    return filePathList;
}

QString getMainClassNameInFile(QString filePath)
{
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";

    bool bExistMainClass = false;
    QStringList packageNameList;
    QStringList classNameList;

    QTextStream in(&file);
    QString line = in.readLine();
    while(!line.isNull()) {
        QString packageName = getPackageName(line);
        if (!packageName.trimmed().isEmpty())
            packageNameList.push_back(packageName);

        QString className = getClassName(line);
        if (!className.trimmed().isEmpty())
            classNameList.push_back(className);

        int pos = line.indexOf(MAINCLASS_PATTERN);
        if( pos > -1) {
            bExistMainClass = true;
            break;
        }

        line = in.readLine();
    }

    if (!bExistMainClass || packageNameList.isEmpty() || classNameList.isEmpty())
        return "";

    QString mainClassName = packageNameList.last() + "." + classNameList.last();
    return mainClassName;
}

QPair<QString, QString> getMainClassNameInDir(QString sourcePath)
{
    QPair<QString, QString> mainClassInfo;
    QStringList sourceFileList = getAllCustomFiles(sourcePath, "java");
    foreach (auto sourceFile, sourceFileList) {
        QString mainClassName = getMainClassNameInFile(sourceFile);
        if (!mainClassName.trimmed().isEmpty()) {
            mainClassInfo = QPair<QString, QString>(mainClassName.trimmed(), sourceFile);
            break;
        }
    }

    return mainClassInfo;
}

} //namespace javaparser
