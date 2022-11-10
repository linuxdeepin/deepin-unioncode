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
#include "projectgenerate.h"

#include <QFile>
#include <QDir>
#include <QTextStream>

namespace templateMgr{

class ProjectGeneratePrivate
{
    friend class ProjectGenerate;
};

ProjectGenerate::ProjectGenerate(QObject *parent)
    : QObject(parent)
    , d(new ProjectGeneratePrivate())
{

}

ProjectGenerate::~ProjectGenerate()
{
    if (d)
        delete d;
}

bool ProjectGenerate::copyDir(QString &retMsg, const QString &srcPath, const QString &dstPath, bool cover)
{
    QDir srcDir(srcPath);
    QDir dstDir(dstPath);
    if (!dstDir.exists()) {
        if (!dstDir.mkdir(dstDir.absolutePath())) {
            retMsg = tr("Create ") + dstPath + tr(" failed.");
            return false;
        }
    }

    QFileInfoList fileInfoList = srcDir.entryInfoList();
    foreach(auto fileInfo, fileInfoList) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir()) {
            if (!copyDir(retMsg, fileInfo.filePath(), dstDir.filePath(fileInfo.fileName()), cover))
                return false;
        } else {
            if (cover && dstDir.exists(fileInfo.fileName())) {
                dstDir.remove(fileInfo.fileName());
            }

            if (!QFile::copy(fileInfo.filePath(), dstDir.filePath(fileInfo.fileName()))) {
                retMsg = tr("Copy file ") + fileInfo.fileName() + tr(" failed.");
                return false;
            }
        }
    }

    return true;
}

bool ProjectGenerate::copyFile(QString &retMsg, const QString &srcPath, const QString &dstPath, bool cover)
{
    if (!QFile::exists(srcPath)) {
        retMsg = tr("Template file ") + srcPath + tr(" is not exist.");
        return false;
    }

    if (QFile::exists(dstPath) && cover) {
        QFile::remove(dstPath);
    }

    QFile::copy(srcPath, dstPath);

    return true;
}

bool ProjectGenerate::create(PojectGenResult &retResult, const PojectGenParam &genParam)
{
    if (genParam.type == Project) {
        return genProject(retResult, genParam);
    } else if (genParam.type == File) {
        return genFile(retResult, genParam);
    } else {
        retResult.message = tr("Template type is unknown.");
        return false;
    }
}

bool ProjectGenerate::genProject(PojectGenResult &retResult, const PojectGenParam &genParam)
{
    if (genParam.templatePath.isEmpty()) {
        retResult.message = tr("Template folder is empty!");
        return false;
    }

    QString dstPath = genParam.settingParamMap.value(genParam.generator.destPath);
    if (dstPath.isEmpty()) {
        retResult.message = tr("Target path is empty!");
        return false;
    }

    QString projectPath = dstPath + QDir::separator() +
            genParam.settingParamMap.value(genParam.generator.rootFolder);

    if (QDir(projectPath).exists()) {
        retResult.message = projectPath + tr(" has existed, please remove it firstly.");
        return false;
    }

    bool ret = copyDir(retResult.message, genParam.templatePath, projectPath, false);
    if (!ret) {
        retResult.message = tr("Create project failed!");
        return false;
    }

    QString wiazardFile = projectPath + QDir::separator() + "wizard.json";
    QFile::remove(wiazardFile);

    ret = transform(retResult.message, genParam, projectPath);
    if (!ret) {
        retResult.message = tr("Transform project failed!") + retResult.message;
        return false;
    }

    retResult.message = tr("succeed");
    retResult.kit = genParam.kit;
    retResult.language = genParam.language;
    retResult.projectPath = projectPath;

    return true;
}

bool ProjectGenerate::transform(QString &retMsg, const PojectGenParam &genParam, const QString &projectPath)
{
    auto iter = genParam.generator.operations.begin();
    for (; iter != genParam.generator.operations.end(); ++iter) {
        QString filePath = projectPath + QDir::separator() + iter->sourceFile;
        if (!QFileInfo(filePath).isFile()) {
            retMsg = filePath + tr(" is not exsit.");
            return false;
        }

        QFile fileRead(filePath);
        QString contentRead;
        if(!fileRead.open(QIODevice::ReadOnly | QIODevice::Text)){
            retMsg = tr("Open ") + filePath + tr(" failed.");
            return false;
        }
        contentRead = fileRead.readAll();
        fileRead.close();

        QStringList contentList = contentRead.split("\n");

        QFile fileWrite(filePath);
        if(fileWrite.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream streamWrite(&fileWrite);
            foreach (auto line, contentList) {
                QString contentWrite = line;
                foreach (auto key, iter->replaceKeys) {
                    if(contentWrite.contains(key)) {
                        contentWrite.replace(key, genParam.settingParamMap.value(key));
                    }
                }
                streamWrite << contentWrite << "\n";
            }
        }
        fileWrite.close();
    }

    return true;
}

bool ProjectGenerate::genFile(PojectGenResult &retResult, const PojectGenParam &genParam)
{
    if (genParam.templatePath.isEmpty()) {
        retResult.message = tr("Template file is empty!");
        return false;
    }

    QString srcFilePath = genParam.templatePath + QDir::separator() + genParam.generator.templateFile;

    QString dstPath = genParam.settingParamMap.value(genParam.generator.destPath);
    if (dstPath.isEmpty()) {
        retResult.message = tr("Target path is empty!");
        return false;
    }

    QString suffix = QFileInfo(srcFilePath).suffix();
    QString newFilePath = dstPath + QDir::separator()
            + genParam.settingParamMap.value(genParam.generator.newfileName)
            + "."
            + suffix;

    if (QFile::exists(newFilePath)) {
        retResult.message = newFilePath + tr(" has existed, please remove it firstly.");
        return false;
    }

    bool ret = copyFile(retResult.message, srcFilePath, newFilePath, false);
    if (!ret) {
        retResult.message = tr("Create File failed!");
        return false;
    }

    retResult.message = tr("succeed");
    retResult.language = genParam.language;
    retResult.filePath = newFilePath;

    return true;
}

} //namespace templateMgr
