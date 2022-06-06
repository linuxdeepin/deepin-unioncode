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
};

#endif // FILEOPERATION_H
