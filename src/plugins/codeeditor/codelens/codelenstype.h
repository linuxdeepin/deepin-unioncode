// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODELENTYPE_H
#define CODELENTYPE_H

#include <qnamespace.h>

enum CodeLensItemRole
{
    LineRole = Qt::ItemDataRole::UserRole + 1,
    TermStartRole,
    TermEndRole
};


#endif // CODELENTYPE_H
