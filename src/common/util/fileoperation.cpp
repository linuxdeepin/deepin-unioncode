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
    bool ret = false;
    if (QFileInfo(filePath).isFile()) {
        ret = QFile(filePath).remove();
    } else {
        ret = QDir(filePath).removeRecursively();
    }
    return ret;
}

QString checkDuplicate(const QString & parentPath, const QString &newFileName)
{
    QString newFilePath = parentPath + QDir::separator() + newFileName;
    if (QFile::exists(newFilePath)) {
        QString newName = newFileName + QString("(1)");
        newFilePath = checkDuplicate(parentPath, newName);
    }

    return newFilePath;
};

bool FileOperation::doNewDocument(const QString &parentPath, const QString &docName)
{
    QFileInfo info(parentPath);
    if (!info.exists() || !info.isDir())
        return false;

    QString newFilePath = checkDuplicate(parentPath, docName);

    QFile newFile(newFilePath);
    if (newFile.open(QFile::OpenModeFlag::NewOnly)){
        newFile.close();
    }
    return true;
}

bool FileOperation::doNewFolder(const QString &parentPath, const QString &folderName)
{
    QFileInfo info(parentPath);
    if (!info.exists() || !info.isDir())
        return false;
    else
        return QDir(parentPath).mkdir(folderName);
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
