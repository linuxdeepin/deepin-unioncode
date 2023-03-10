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
#include "clangcursor.h"


ClangCursor::ClangCursor(const CXCursor &_cursor)
    : cursor(_cursor)
{
}

QString toCString(const CXString &cxString)
{
    std::string cstring = clang_getCString(cxString);
    clang_disposeString(cxString);
    return cstring.c_str() == nullptr ? "" : cstring.c_str();
}

QString ClangCursor::kindName() const
{
    CXCursorKind cursorKind = clang_getCursorKind(cursor);
    CXString cxKindName = clang_getCursorKindSpelling(cursorKind);
    return toCString(cxKindName);
}

QString ClangCursor::spelling() const
{
    CXString cxCursorSpelling = clang_getCursorSpelling(cursor);
    return toCString(cxCursorSpelling);
}

QString ClangCursor::displayName() const
{
    CXString diaplay = clang_getCursorDisplayName(cursor);
    return toCString(diaplay);
}

void ClangCursor::location(QString &file, uint &line, uint &column) const
{
    CXSourceLocation location = clang_getCursorLocation(cursor);

    CXString cxFilename;
    clang_getPresumedLocation(location, &cxFilename, &line, &column);
    file = toCString(cxFilename);
}

QString ClangCursor::typeName() const
{
    CXType type = clang_getCursorType(cursor);
    CXString cxTypedefName = clang_getTypedefName(type);
    return toCString(cxTypedefName);
}

QString ClangCursor::typeSpelling() const
{
    CXType type = clang_getCursorType(cursor);
    CXString cxSpelling = clang_getTypeSpelling(type);
    return toCString(cxSpelling);
}

QString ClangCursor::typeKindName() const
{
    CXType cursorType = clang_getCursorType(cursor);
    auto kindName = toCString(clang_getTypeKindSpelling(cursorType.kind));
    return kindName;
}

ClangCursor ClangCursor::sematicParent() const
{
    return ClangCursor(clang_getCursorSemanticParent(cursor));
}

QString ClangCursor::translationUnitSpelling() const
{
    CXTranslationUnit unit = clang_Cursor_getTranslationUnit(cursor);
    CXString unitSpelling =clang_getTranslationUnitSpelling(unit);
    return toCString(unitSpelling);
}

bool ClangCursor::isValid() const
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    return !clang_isInvalid(kind);
}
