/**
 * C/C++ Debug Adaptor Protocol Server drived by google cppdap library
 *
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xiaozaihu<xiaozaihu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             xiaozaihu<xiaozaihu@uniontech.com>
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

#ifndef PROTOCOL_EXTEND_H
#define PROTOCOL_EXTEND_H

#include "dap/protocol.h"
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

namespace  dap {

// extend dap::LaunchRequest
class LaunchRequestExtend : public LaunchRequest
{
public:
    dap::string name;
    dap::string type;
    dap::string program;
};

DAP_STRUCT_TYPEINFO_EXT(LaunchRequestExtend,
                        LaunchRequest,
                        "launch",
                        DAP_FIELD(name, "name"),
                        DAP_FIELD(type, "type"),
                        DAP_FIELD(program, "program"));
} //namespace dap

#endif // PROTOCOL_EXTEND_H
