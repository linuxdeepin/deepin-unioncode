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

namespace  dap {

struct setupCommand
{
    optional<string> description;
    optional<string> text;
    optional<boolean> ignoreFailures;
};

struct logging
{
    optional<boolean> trace;
    optional<boolean> traceResponse;
    optional<boolean> engineLogging;
};

class LaunchRequestExtend : public LaunchRequest
{
public:
    optional<string> name;
    optional<string> type;
    optional<string> request;
    optional<string> program;
    optional<array<string>> args;
    optional<boolean> stopAtEntry;
    optional<string> cwd;
    optional<array<string>> environment;
    optional<boolean> externalConsole;
    optional<string> MIMode;
    //optional<array<setupCommand>> setupCommands;
    //optional<logging> logging;
    optional<integer> __configurationTarget;
    optional<string> __sessionId;
};

DAP_STRUCT_TYPEINFO_EXT(LaunchRequestExtend,
                        LaunchRequest,
                        "launch",
                        DAP_FIELD(name, "name"),
                        DAP_FIELD(type, "type"),
                        DAP_FIELD(request, "request"),
                        DAP_FIELD(program, "program"),
                        DAP_FIELD(args, "args"),
                        DAP_FIELD(stopAtEntry, "stopAtEntry"),
                        DAP_FIELD(environment, "environment"),
                        DAP_FIELD(cwd, "cwd"),
                        DAP_FIELD(externalConsole, "externalConsole"),
                        DAP_FIELD(MIMode, "MIMode"),
                        //DAP_FIELD(setupCommands, "setupCommands"),
                        //DAP_FIELD(logging, "logging"),
                        DAP_FIELD(__configurationTarget, "__configurationTarget"),
                        DAP_FIELD(__sessionId, "__sessionId"));
} //namespace dap

#endif // PROTOCOL_EXTEND_H
