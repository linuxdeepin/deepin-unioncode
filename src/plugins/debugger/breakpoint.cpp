// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
