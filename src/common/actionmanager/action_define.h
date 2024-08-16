// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACTION_DEFINE_H
#define ACTION_DEFINE_H

#include <QString>

// Menus
constexpr char M_TITLEBAR[] = "IDE.Menu.Titlebar";   // main title menu
constexpr char M_FILE[] = "IDE.Menu.File";
constexpr char M_EDIT[] = "IDE.Menu.Edit";
constexpr char M_BUILD[] = "IDE.Menu.Build";
constexpr char M_DEBUG[] = "IDE.Menu.Debug";
constexpr char M_TOOLS[] = "IDE.Menu.Tools";
constexpr char M_TOOLS_BINARY[] = "IDE.Menu.Tools.Binary";
constexpr char M_TOOLS_REVERSEDEBUG[] = "IDE.Menu.Tools.ReverseDebug";
constexpr char M_HELP[] = "IDE.Menu.Help";

// Contexts
constexpr char C_GLOBAL[] = "Global Context";

// Special context that leads to all "more specific" contexts to be ignored.
// If you use Context(mycontextId, C_GLOBAL_CUTOFF) for a widget that has focus,
// mycontextId will be enabled but the contexts for all parent widgets, the manually added
// "additional" contexts, and the global context will be turned off.
constexpr char C_GLOBAL_CUTOFF[] = "Global Cutoff";

// Action Id
constexpr char A_REPORTBUG[] = "IDE.Action.ReportBug";
constexpr char A_HELPDOC[] = "IDE.Action.HelpDocument";
constexpr char A_TOOLBUTTON[] = "IDE.Action.ToolButton";
constexpr char A_OPEN_FILE[] = "IDE.Action.OpenFile";
constexpr char A_OPEN_PROJECT[] = "IDE.Action.OpenProject";

// Default groups
constexpr char G_DEFAULT_ONE[] = "IDE.Group.Default.One";
constexpr char G_DEFAULT_TWO[] = "IDE.Group.Default.Two";
constexpr char G_DEFAULT_THREE[] = "IDE.Group.Default.Three";

// Main title bar menu groups
constexpr char G_FILE[] = "IDE.Group.File";
constexpr char G_EDIT[] = "IDE.Group.Edit";
constexpr char G_BUILD[] = "IDE.Group.Build";
constexpr char G_DEBUG[] = "IDE.Group.Debug";
constexpr char G_TOOLS[] = "IDE.Group.Tools";
constexpr char G_HELP[] = "IDE.Group.Help";

// File menu groups
constexpr char G_FILE_NEW[] = "IDE.Group.File.New";
constexpr char G_FILE_OPEN[] = "IDE.Group.File.Open";
constexpr char G_FILE_CLOSE[] = "IDE.Group.File.Close";
constexpr char G_FILE_SAVE[] = "IDE.Group.File.Save";

// Edit menu groups
constexpr char G_EDIT_UNDOREDO[] = "IDE.Group.Edit.UndoRedo";
constexpr char G_EDIT_COPYPASTE[] = "IDE.Group.Edit.CopyPaste";
constexpr char G_EDIT_SELECTALL[] = "IDE.Group.Edit.SelectAll";
constexpr char G_EDIT_FIND[] = "IDE.Group.Edit.Find";
constexpr char G_EDIT_OTHER[] = "IDE.Group.Edit.Other";

inline QString stripAccelerator(const QString &text)
{
    QString res = text;
    for (int index = res.indexOf('&'); index != -1; index = res.indexOf('&', index + 1))
        res.remove(index, 1);
    return res;
}

#endif   // ACTION_DEFINE_H
