// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODELENTYPE_H
#define CODELENTYPE_H

#include <qnamespace.h>

enum CodeLensItemRole
{
    Range = Qt::ItemDataRole::UserRole + 1,
    CodeText,
    HeightColor
};


#endif // CODELENTYPE_H
