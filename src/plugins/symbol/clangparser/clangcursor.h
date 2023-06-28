// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
