// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMITDATAROLE_H
#define COMMITDATAROLE_H

#include <QMetaEnum>
#include "common/common.h"

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

    bool isSameFilePath(const RevisionFile &file) const {
        return filePath == file.filePath;
    }

    bool isSameReviType(const RevisionFile &file) const {
        return revisionType == file.revisionType;
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

// 第一列表示添加、删除或以其他方式更改的项目：
enum_def(AmendsState_Col1, QString)
{
    enum_exp NUL = " "; // 没有修改。
    enum_exp SRC = "?"; // 项目不受版本控制。
    enum_exp ADD = "A"; // 项目计划添加。
    enum_exp DEL = "D"; // 项目计划删除。
    enum_exp MOD = "M"; // 项目已修改。
    enum_exp REP = "R"; // 您的工作副本中的项目已被替换。这意味着该文件被安排删除，然后一个同名的新文件被安排添加到它的位置。
    enum_exp CON = "C"; // 项目的内容（相对于属性）与从存储库接收的更新冲突。
    enum_exp EXT = "X"; // 由于外部定义，项目存在。
    enum_exp IGN = "I"; // 项目被忽略（例如，与 svn:ignore属性）。
    enum_exp LOS = "!"; // 项目丢失（例如，您在没有使用svn的情况下移动或删除了它）。这也表明目录不完整（签出或更新被中断）。
    enum_exp OTH = "~"; // 项目被版本化为一种对象（文件、目录、链接），但已被另一种对象替换。
};

// 第二列告诉文件或目录属性的状态：
enum_def(AmendsState_Col2, QString)
{
    enum_exp NUL = " "; // 没有修改。
    enum_exp MOD = "M"; // 此项目的属性已被修改。
    enum_exp CON = "C"; // 此项目的属性与从存储库接收的属性更新冲突。
};

// 仅当工作副本目录被锁定时才会填充第三列：
enum_def(AmendsState_Col3, QString)
{
    enum_exp NUL = " "; // 项目未锁定。
    enum_exp LOC = "L"; // 项目已锁定。
};

// 仅当项目计划添加历史记录时才会填充第四列：
enum_def(AmendsState_Col4, QString)
{
    enum_exp NUL = " "; // 没有安排提交的历史记录。
    enum_exp LOC = "+"; // 与提交一起安排的历史记录。
};

// 仅当项相对于其父项切换时才填充第五列（请参阅名为“遍历分支”的部分）：
enum_def(AmendsState_Col5, QString)
{
    enum_exp NUL = " "; // Item 是其父目录的子目录。
    enum_exp SWI = "S"; // 项目已切换。
};

// 第六列填充了锁定信息：
enum_def(AmendsState_Col6, QString)
{
    enum_exp NUL = " "; // 当使用--show-updates ( -u) 时，这意味着文件没有被锁定。如果--show-updates ( -u)没有 被使用，这仅仅意味着文件没有被锁定在这个工作副本中。
    enum_exp KLOC = "K"; // 文件被锁定在这个工作副本中。
    enum_exp OLOC = "O"; // 文件被另一个用户或另一个工作副本锁定。这仅在 使用--show-updates ( -u) 时出现。
    enum_exp TLOC = "T"; // 文件被锁定在此工作副本中，但锁定已“被盗”且无效。该文件当前已锁定在存储库中。这仅在使用--show-updates ( -u) 时出现。
    enum_exp BLOC = "B"; // 文件被锁定在此工作副本中，但锁定已“损坏”且无效。该文件不再被锁定。这仅在使用--show-updates ( -u) 时出现。
};

// 仅当项目是树冲突的受害者时才填充第七列：
enum_def(AmendsState_Col7, QString)
{
    enum_exp NUL = " "; // 项目不是树冲突的受害者。
    enum_exp CON = "C"; // 项目是树冲突的受害者。
};

// 第八列始终为空白。
enum_def(AmendsState_Col8, QString)
{
    enum_exp NUL = " ";
};

// 过期信息出现在第九列（仅当您通过 --show-updates( -u) 选项时）：
enum_def(AmendsState_Col9, QString)
{
    enum_exp NUL = " "; // 您的工作副本中的项目是最新的。
    enum_exp NEW = "*"; // 服务器上存在该项目的较新版本。
};

inline static QString svnProgram()
{
    QString program("/usr/bin/svn");
    QFileInfo fileInfo(program);
    if (!fileInfo.exists() || !fileInfo.isExecutable()) {
        CommonDialog::ok(QDialog::tr("Please install \"subversion\" software from terminal:"
                                      "$ sudo apt install subversion"));
        return "";
    }
    return program;
}

inline static QString svnLogSplitStr()
{
    return "------------------------------------"
           "------------------------------------\n";
}

#endif // COMMITDATAROLE_H
