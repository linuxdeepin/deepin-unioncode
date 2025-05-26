// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <QString>
#include <QAction>

namespace SymbolPri{
static QString recordFileName {".record"};
static QString definitionsFileName {".definitions"};
static QString declaredFileName {".declared"};
static QString declaredAcStr {QAction::tr("jump to declared")};
static QString recordAcStr {QAction::tr("jump to record")};
static QString definitionsAcStr {QAction::tr("jump to deninitions")};
}
#endif // DEFINITIONS_H
