// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include "common/itemnode.h"

enum ItemRole {
    ItemStateRole = Qt::UserRole + 1
};

class Utils
{
public:
    static ProjectNode *createProjectNode(const dpfservice::ProjectInfo &info);

    static FolderNode *recursiveFindOrCreateFolderNode(FolderNode *folder,
                                                       const QString &directory,
                                                       const QString &workspace,
                                                       const FolderNode::FolderNodeFactory &factory);
    static bool isChildOf(const QString &path, const QString &subPath);
    static QString relativeChildPath(const QString &path, const QString &subPath);
    static bool isValidPath(const QString &path);
    static bool isCppFile(const QString &filePath);
    static bool isCMakeFile(const QString &filePath);
    static QStringList relateFileList(const QString &filePath);
    static QString createUTFile(const QString &workspace, const QString &filePath,
                                const QString &target, const QString &nameFormat);

    static QString createRequestPrompt(const FileNode *node, const QString &chunkPrompt, const QString &userPrompt);
    static QString createChunkPrompt(const QJsonObject &chunkObj);
    static QString createCMakePrompt(const QString &testFramework);
    static QStringList queryCodePart(const QString &content, const QString &type);
    static bool checkAnyState(NodeItem *item, ItemState state);
    static bool checkAllState(NodeItem *item, ItemState state);
};

#endif   // UTILS_H
