// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
