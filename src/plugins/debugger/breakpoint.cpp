/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "breakpoint.h"

namespace Internal
{
Breakpoint::Breakpoint()
{

}

void Breakpoint::update(dap::Breakpoint &bp)
{
    if (bp.source.has_value() && bp.source.value().name.has_value()
            && bp.source.value().path.has_value()) {
        fileName = bp.source.value().name.value().c_str();
        filePath = bp.source.value().path.value().c_str();
    }

    if (bp.line.has_value()) {
        lineNumber = static_cast<int>(bp.line.value());
    }
}
} // end namespace.
