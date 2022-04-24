/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#ifndef COMMITDATAROLE_H
#define COMMITDATAROLE_H

#include <QMetaEnum>

enum CommitHistoryRole
{
    TreeViewIcon = Qt::ItemDataRole::UserRole,
    Graph,
    Log,
    Author,
    Date,
    Sha
};

enum FileModifyRole
{
    FilePath = Qt::ItemDataRole::UserRole,
    FileIconType,
    RevisionType,
};

struct RevisionFile
{
    QString displayName;
    QString filePath;
    QString revisionType;

    RevisionFile(){}

    RevisionFile(const QString displayName,
                 const QString filePath,
                 const QString revisionType)
        : displayName(displayName)
        , filePath(filePath)
        , revisionType(revisionType)
    {}

    bool isInvalid() {
        if (displayName.isEmpty() || filePath.isEmpty()
                || revisionType.isEmpty())
            return true;
        return false;
    }

    bool operator == (const RevisionFile &file) {
        return displayName == file.displayName
                && filePath == file.filePath
                && revisionType == file.revisionType;
    }
};

typedef QList<RevisionFile> RevisionFiles;

#endif // COMMITDATAROLE_H
