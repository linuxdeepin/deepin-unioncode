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

#ifndef REVERSE_DEBUGGER_CONSTANTS_H
#define REVERSE_DEBUGGER_CONSTANTS_H

namespace ReverseDebugger {
namespace Constants {

const char RECORD_ID[] = "ReverseDebugger.Record";
const char REPLAY_ID[] = "ReverseDebugger.Replay";
const char STOP_ID[] = "ReverseDebugger.Stop";
const char MINIDUMP_RECORD[] = "ReverseDebugger.MinidumpRecord";
const char MINIDUMP_VIEW[] = "ReverseDebugger.MinidumpView";
const char MENU_ID[] = "ReverseDebugger.Menu";
const char RUN_MODE_MINIDUMP[] = "ReverseDebugger.Minidump";
const char RUN_MODE_RECORD[] = "ReverseDebugger.RunUnderRR";
const char RUN_MODE_REPLAY[] = "ReverseDebugger.AttachToRR";

const char EVENT_CATEGORY_SYSCALL[] = "Event.Category.Syscall";
const char EVENT_CATEGORY_SIGNAL[] = "Event.Category.Signal";
const char EVENT_CATEGORY_X11[] = "Event.Category.X11Event";
const char EVENT_CATEGORY_DBUS[] = "Event.Category.DbusMsg";

} // namespace ReverseDebugger
} // namespace Constants

#endif // REVERSE_DEBUGGER_CONSTANTS_H
