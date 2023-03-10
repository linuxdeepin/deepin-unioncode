/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef CLANGCURSOR_H
#define CLANGCURSOR_H

#include <QObject>
#include <clang-c/Index.h>

class ClangCursor
{
public:
    explicit ClangCursor(const CXCursor &cursor);

    QString kindName() const;
    QString spelling() const;
    QString displayName() const;
    void location(QString &file, uint &line, uint &column) const;
    QString typeName() const;
    QString typeSpelling() const;
    QString typeKindName() const;
    ClangCursor sematicParent() const;
    QString translationUnitSpelling() const;
    bool isValid() const;

private:
    CXCursor cursor;
};

#endif // CLANGCURSOR_H
