/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef SUPPORTFILE_H
#define SUPPORTFILE_H

#include <QString>
#include <QJsonObject>

namespace support_file
{

struct BuildFileInfo
{
    QString buildSystem;
    QString projectPath;
    bool operator==(const BuildFileInfo &info) const;
    bool isEmpty();
};

struct Builder
{
    static QString globalPath();
    static QString userPath();
    static void initialize();
    static QString buildSystem(const QString &filePath);
    static BuildFileInfo buildInfo(const QString &filePath);
    // 多线程线程执行
    static QList<BuildFileInfo> buildInfos(const QString &dirPath);
    static bool load();
};

uint qHash(const support_file::BuildFileInfo &info, uint seed = 0);
}


#endif // SUPPORTFILE_H
