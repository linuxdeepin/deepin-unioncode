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
