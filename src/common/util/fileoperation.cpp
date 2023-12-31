// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileoperation.h"
#include "processutil.h"
#include "common.h"

#include <QDialog>
#include <QDir>
#include <QFileInfo>

const QString DELETE_MESSAGE_TEXT {QDialog::tr("The delete operation will be removed from"
                                               "the disk and will not be recoverable "
                                               "after this operation.\nDelete anyway?")};

const QString DELETE_WINDOW_TEXT {QDialog::tr("Delete Warning")};

bool FileOperation::doMoveMoveToTrash(const QString &filePath)
{
    return ProcessUtil::moveToTrash(filePath);
}

bool FileOperation::doRecoverFromTrash(const QString &filePath)
{
    return ProcessUtil::recoverFromTrash(filePath);
}

bool FileOperation::doRemove(const QString &filePath)
{
    return QFile(filePath).remove();
}

bool FileOperation::doNewDocument(const QString &parentPath, const QString &docName)
{
    QFileInfo info(parentPath);
    if (!info.exists() || !info.isDir())
        return false;
    else  {
        QFile file(parentPath + QDir::separator() + docName);
        if (file.open(QFile::OpenModeFlag::NewOnly)){
            file.close();
        }
        return false;
    }
}

bool FileOperation::doNewFolder(const QString &parentPath, const QString &folderName)
{
    QFileInfo info(parentPath);
    if (!info.exists() || !info.isDir())
        return false;
    else
        return QDir(parentPath).mkdir(folderName);
}

bool FileOperation::deleteDir(const QString &path)
{
    if (path.isEmpty()){
        return true;
    }

    QDir dir(path);
    if(!dir.exists()){
        return true;
    }

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    foreach (auto file, fileList) {
        if (file.isFile())
            file.dir().remove(file.fileName());
        else
            deleteDir(file.absoluteFilePath());
    }

    return dir.rmdir(dir.absolutePath());
}

QString FileOperation::checkCreateDir(const QString &src, const QString &dirName)
{
    QDir dir(src);
    if (dir.exists()) {
        if (!dir.exists(dirName)) {
            dir.mkdir(dirName);
        }
        dir.cd(dirName);
    }
    return dir.path();
}

QString FileOperation::readAll(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        qCritical() << file.errorString();
        return "";
    }

    QString ret = file.readAll();
    file.close();
    return ret;
}

bool FileOperation::exists(const QString &filePath)
{
    return QFileInfo(filePath).exists();
}
