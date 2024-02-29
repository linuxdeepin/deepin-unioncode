// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QString>

class FileOperation final
{
public:
    static bool doMoveMoveToTrash(const QString &filePath);
    static bool doRecoverFromTrash(const QString &filePath);
    static bool doRemove(const QString &filePath);
    static bool doNewDocument(const QString &parentPath, const QString &docName);
    static bool doNewFolder(const QString &parentPath, const QString &folderName);
    static QString checkCreateDir(const QString &src, const QString &dirName);
    static QString readAll(const QString &filePath);
    static bool exists(const QString &filePath);
};

#endif // FILEOPERATION_H
