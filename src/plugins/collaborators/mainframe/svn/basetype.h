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

enum FileModifyRole
{
    FilePathRole = Qt::ItemDataRole::UserRole,
    FileIconTypeRole,
    RevisionTypeRole,
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

    bool isValid() const {
        if (!displayName.isEmpty() || !filePath.isEmpty()
                || !revisionType.isEmpty())
            return true;
        return false;
    }

    bool operator == (const RevisionFile &file) const {
        return displayName == file.displayName
                && filePath == file.filePath
                && revisionType == file.revisionType;
    }
};

typedef QList<RevisionFile> RevisionFiles;

Q_DECLARE_METATYPE(RevisionFile)
Q_DECLARE_METATYPE(RevisionFiles)


enum HistoryRole
{
    DescriptionRole = Qt::ItemDataRole::UserRole,
    RevisionFilesRole,
};

struct HistoryData
{
    QString revision;
    QString user;
    QString dateTime;
    QString lineCount;
    QString description;
    RevisionFiles changedFiles;

    HistoryData(){}

    HistoryData(const QString &revision,
                const QString &user,
                const QString &dateTime,
                const QString &lineCount,
                const QString &description = "",
                const RevisionFiles &changedFiles= {})
        : revision(revision)
        , user(user)
        , dateTime(dateTime)
        , lineCount(lineCount)
        , description(description)
        , changedFiles(changedFiles)
    {

    }

    bool isValid() const {
        if (!revision.isEmpty() || !user.isEmpty()
                || !dateTime.isEmpty() || !lineCount.isEmpty())
            return true;
        return false;
    }

    bool operator == (const HistoryData &data) const {
        return revision == data.revision
                && user == data.user
                && dateTime == data.dateTime
                && lineCount == data.lineCount
                && description == data.description
                && changedFiles == data.changedFiles;
    }
};

typedef QList<HistoryData> HistoryDatas;

#endif // COMMITDATAROLE_H
